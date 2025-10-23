// UniformBuffer.h

#pragma once

#include "Resource.h" // �θ� Ŭ���� ����
#include <vulkan/vulkan.h>

class UniformBuffer : public Resource {
public:

    UniformBuffer(const VulkanContext* context, VkDeviceSize size);
    ~UniformBuffer();

    void update(const void* data);

    VkBuffer getBuffer() const { return buffer_; }
    VkDescriptorBufferInfo GetDescriptorBufferInfo() const {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer_;
        bufferInfo.offset = 0;
        bufferInfo.range = bufferSize_;
        return bufferInfo;
	}

private:
    VkBuffer       buffer_ = VK_NULL_HANDLE;
    VkDeviceMemory bufferMemory_ = VK_NULL_HANDLE;
    VkDeviceSize   bufferSize_ = 0;
};