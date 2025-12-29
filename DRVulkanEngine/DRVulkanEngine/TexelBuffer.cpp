#include "TexelBuffer.h"
#include "VulkanContext.h"
#include <stdexcept>
#include <cstring>

TexelBuffer::TexelBuffer(const VulkanContext* ctx, VkDeviceSize size, VkFormat format)
    : context_(ctx), bufferSize_(size), format_(format) {

    // 1. 버퍼 생성 (USAGE에 TEXEL_BUFFER_BIT가 들어갑니다)
    createBuffer(bufferSize_,
        VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        buffer_,
        bufferMemory_);

    // 2. VkBufferView 생성 (이것이 TMU 하드웨어와 연결되는 통로입니다)
    VkBufferViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    viewInfo.buffer = buffer_;
    viewInfo.format = format_; // 여기서 지정한 포맷으로 TMU가 자동 변환(Expand at Load)함
    viewInfo.offset = 0;
    viewInfo.range = bufferSize_;

    if (vkCreateBufferView(context_->getDevice(), &viewInfo, nullptr, &bufferView_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer view!");
    }
}

TexelBuffer::~TexelBuffer() {
    if (bufferView_ != VK_NULL_HANDLE) {
        vkDestroyBufferView(context_->getDevice(), bufferView_, nullptr);
    }
    vkDestroyBuffer(context_->getDevice(), buffer_, nullptr);
    vkFreeMemory(context_->getDevice(), bufferMemory_, nullptr);
}

void TexelBuffer::populateWriteDescriptor(VkWriteDescriptorSet& writeInfo) const {
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.descriptorCount = 1;
    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER; // TMU 경로 사용 명시

    // 중요: 텍셀 버퍼는 pBufferInfo가 아니라 pTexelBufferView를 사용합니다.
    writeInfo.pTexelBufferView = &bufferView_;
    writeInfo.pBufferInfo = nullptr;
    writeInfo.pImageInfo = nullptr;
}

void TexelBuffer::update(const void* data) {
    void* mappedMemory;
    vkMapMemory(context_->getDevice(), bufferMemory_, 0, bufferSize_, 0, &mappedMemory);
    memcpy(mappedMemory, data, static_cast<size_t>(bufferSize_));
    vkUnmapMemory(context_->getDevice(), bufferMemory_);
}