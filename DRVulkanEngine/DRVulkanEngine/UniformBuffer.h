#pragma once
#include "Resource.h"
#include <vulkan/vulkan.h>

class VulkanContext;

class UniformBuffer : public Resource {
public:
    UniformBuffer(const VulkanContext* context, VkDeviceSize size);
    ~UniformBuffer();

    void update(const void* data);
    VkBuffer getBuffer() const { return buffer_; }
	VkDescriptorBufferInfo getBufferInfo() const { return bufferInfo_; }
    virtual void populateWriteDescriptor(VkWriteDescriptorSet& writeInfo) const override;

private:
    uint32_t binding_;
    VkBuffer buffer_ = VK_NULL_HANDLE;
    VkDeviceMemory bufferMemory_ = VK_NULL_HANDLE;
    VkDeviceSize bufferSize_ = 0;

    VkDescriptorBufferInfo bufferInfo_;

    const VulkanContext* context_;
};