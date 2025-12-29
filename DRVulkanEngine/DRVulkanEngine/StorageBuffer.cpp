#include "StorageBuffer.h"
#include "VulkanContext.h"
#include <stdexcept>
#include <cstring>

StorageBuffer::StorageBuffer(const VulkanContext* ctx, VkDeviceSize size)
    : context_(ctx), bufferSize_(size) {

    // 1. 버퍼 생성 
    // USAGE에 STORAGE_BUFFER_BIT와 SHADER_DEVICE_ADDRESS_BIT를 추가합니다.
    // 이를 통해 일반적인 디스크립터 방식과 현대적인 포인터 방식을 모두 지원합니다.
    createBuffer(bufferSize_,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        buffer_,
        bufferMemory_);

    // 2. 디스크립터 정보 설정
    bufferInfo_.buffer = buffer_;
    bufferInfo_.offset = 0;
    bufferInfo_.range = bufferSize_;

    // 3. 현대적 설계의 핵심: GPU 내 실제 메모리 주소(64비트)를 가져옵니다.
    VkBufferDeviceAddressInfo addressInfo{};
    addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    addressInfo.buffer = buffer_;
    deviceAddress_ = vkGetBufferDeviceAddress(context_->getDevice(), &addressInfo);
}

StorageBuffer::~StorageBuffer() {
    if (buffer_ != VK_NULL_HANDLE) {
        vkDestroyBuffer(context_->getDevice(), buffer_, nullptr);
    }
    if (bufferMemory_ != VK_NULL_HANDLE) {
        vkFreeMemory(context_->getDevice(), bufferMemory_, nullptr);
    }
}

void StorageBuffer::update(const void* data) {
    void* mappedMemory;
    vkMapMemory(context_->getDevice(), bufferMemory_, 0, bufferSize_, 0, &mappedMemory);
    memcpy(mappedMemory, data, static_cast<size_t>(bufferSize_));
    vkUnmapMemory(context_->getDevice(), bufferMemory_);
}

void StorageBuffer::populateWriteDescriptor(VkWriteDescriptorSet& writeInfo) const {
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.descriptorCount = 1;
    // 유니폼 버퍼와 다른 STORAGE_BUFFER 타입을 명시합니다.
    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeInfo.pBufferInfo = &bufferInfo_;
    writeInfo.pImageInfo = nullptr;
    writeInfo.pTexelBufferView = nullptr;
}