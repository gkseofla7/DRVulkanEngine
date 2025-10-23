#pragma once
#include <vulkan/vulkan.h>
class VulkanContext;
class VulkanSwapChain;
class Resource
{

protected:
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
protected:
	const VulkanContext* context;
	const VulkanSwapChain* swapChain;
};

