#pragma once
#include "Resource.h"
#include <string>
class Texture : public Resource
{
public:
	Texture(const class VulkanContext* context, const std::string& filepath);
	Texture(const class VulkanContext* context, uint32_t width, uint32_t height,
		VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);
	~Texture();

	// Getter 함수들
	VkImage getImage() const { return texture_; }
	VkImageView getImageView() const { return textureView_; }
	VkSampler getSampler() const { return textureSampler_; }
	VkDescriptorImageInfo getImageInfo() const { return imageInfo_; }
	virtual void populateWriteDescriptor(VkWriteDescriptorSet& writeInfo) const override;
	void transitionLayout_Cmd(VkCommandBuffer commandBuffer, VkImageLayout newLayout);

	VkImageLayout getImageLayout() const { return imageInfo_.imageLayout; }
private:
	void initialize(const std::string& filepath);
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);
	void createTextureSampler();
private:
	VkImage texture_;
	VkDeviceMemory textureMemory_;
	VkImageView textureView_;
	VkSampler textureSampler_;

	mutable VkDescriptorImageInfo imageInfo_;

	VkImageLayout currentLayout_ = VK_IMAGE_LAYOUT_UNDEFINED;
	VkFormat format_;
};

