#pragma once
#include "Resource.h"
#include <vulkan/vulkan.h>

class TexelBuffer : public Resource {
public:
    // 텍셀 버퍼는 '포맷(VkFormat)' 정보가 필수입니다 (TMU가 변환에 사용)
    TexelBuffer(const VulkanContext* context, VkDeviceSize size, VkFormat format);
    ~TexelBuffer();

    void update(const void* data);
    virtual void populateWriteDescriptor(VkWriteDescriptorSet& writeInfo) const override;

private:
    VkBuffer buffer_ = VK_NULL_HANDLE;
    VkDeviceMemory bufferMemory_ = VK_NULL_HANDLE;
    VkBufferView bufferView_ = VK_NULL_HANDLE; // TMU 경로를 위한 핵심 객체
    VkDeviceSize bufferSize_ = 0;
    VkFormat format_;

    const VulkanContext* context_;
};