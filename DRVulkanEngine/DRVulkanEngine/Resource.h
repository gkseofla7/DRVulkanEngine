#pragma once
#include <vulkan/vulkan.h>
class VulkanContext;
class VulkanSwapChain;
class Resource
{
public:
	virtual void populateWriteDescriptor(VkWriteDescriptorSet& writeInfo) const = 0;
	bool IsBufferInfosDirty() const { return bBufferInfosDirty_; }
	void ClearBufferInfosDirty() { bBufferInfosDirty_ = false; }
protected:
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);


protected:
	const VulkanContext* context;
	const VulkanSwapChain* swapChain;

	bool bBufferInfosDirty_ = false;
};

