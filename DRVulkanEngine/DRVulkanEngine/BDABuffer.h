#pragma once
#include <vulkan/vulkan.h>

class VulkanContext;

class BDABuffer {
public:
    BDABuffer(const VulkanContext* context, VkDeviceSize size, VkBufferUsageFlags usage = 0);
    ~BDABuffer();

    void update(const void* data);

    VkDeviceAddress getDeviceAddress() const { return deviceAddress_; }
    VkBuffer getBuffer() const { return buffer_; }

private:
    VkBuffer buffer_ = VK_NULL_HANDLE;
    VkDeviceMemory bufferMemory_ = VK_NULL_HANDLE;
    VkDeviceSize bufferSize_ = 0;
    VkDeviceAddress deviceAddress_ = 0;

    const VulkanContext* context_;
};