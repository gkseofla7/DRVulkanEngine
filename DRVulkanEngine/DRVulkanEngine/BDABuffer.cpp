#include "BDABuffer.h"
#include "VulkanContext.h"
#include <stdexcept>
#include <cstring>

BDABuffer::BDABuffer(const VulkanContext* ctx, VkDeviceSize size, VkBufferUsageFlags usage)
    : context_(ctx), bufferSize_(size) {

    VkBufferUsageFlags fullUsage = usage | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = size;
    bufferInfo.usage = fullUsage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(context_->getDevice(), &bufferInfo, nullptr, &buffer_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create BDA buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context_->getDevice(), buffer_, &memRequirements);

    // BDA를 쓰기 위해서는 메모리 할당 시 '할당 플래그'를 명시해야 합니다.
    VkMemoryAllocateFlagsInfo flagsInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO };
    flagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT; // 필수 플래그

    VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.pNext = &flagsInfo;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = context_->findMemoryType(memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(context_->getDevice(), &allocInfo, nullptr, &bufferMemory_) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate BDA buffer memory!");
    }

    vkBindBufferMemory(context_->getDevice(), buffer_, bufferMemory_, 0);

    VkBufferDeviceAddressInfo addressInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
    addressInfo.buffer = buffer_;
    deviceAddress_ = vkGetBufferDeviceAddress(context_->getDevice(), &addressInfo);
}

BDABuffer::~BDABuffer() {
    if (buffer_ != VK_NULL_HANDLE) vkDestroyBuffer(context_->getDevice(), buffer_, nullptr);
    if (bufferMemory_ != VK_NULL_HANDLE) vkFreeMemory(context_->getDevice(), bufferMemory_, nullptr);
}

void BDABuffer::update(const void* data) {
    void* mappedMemory;
    vkMapMemory(context_->getDevice(), bufferMemory_, 0, bufferSize_, 0, &mappedMemory);
    memcpy(mappedMemory, data, static_cast<size_t>(bufferSize_));
    vkUnmapMemory(context_->getDevice(), bufferMemory_);
}