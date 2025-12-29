#pragma once
#include "Resource.h"
#include <vulkan/vulkan.h>

class VulkanContext;

class StorageBuffer : public Resource {
public:
    // 일반적인 SSBO 생성 및 BDA(포인터) 기능 활성화
    StorageBuffer(const VulkanContext* context, VkDeviceSize size);
    ~StorageBuffer();

    void update(const void* data);

    // 현대적 방식: 셰이더에서 직접 접근할 수 있는 64비트 주소 반환
    VkDeviceAddress getDeviceAddress() const { return deviceAddress_; }

    VkBuffer getBuffer() const { return buffer_; }
    virtual void populateWriteDescriptor(VkWriteDescriptorSet& writeInfo) const override;

private:
    VkBuffer buffer_ = VK_NULL_HANDLE;
    VkDeviceMemory bufferMemory_ = VK_NULL_HANDLE;
    VkDeviceSize bufferSize_ = 0;
    VkDeviceAddress deviceAddress_ = 0; // 64비트 GPU 주소 저장

    VkDescriptorBufferInfo bufferInfo_;
    const VulkanContext* context_;
};