#pragma once
#include "Resource.h"
#include <string>
class Texture : public Resource
{
public:
	Texture(const class VulkanContext* context, const std::string& filepath);
	// �Ҹ���: �ؽ�ó ���� Vulkan ���ҽ��� �����մϴ�.
	~Texture();

	// Getter �޼����
	VkImageView getImageView() const { return textureView_; }
	VkSampler getSampler() const { return textureSampler_; }
private:
	void initialize(const std::string& filepath);
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	VkImageView createImageView(VkImage image, VkFormat format);
	void createTextureSampler();
private:
	VkImage texture_;
	VkDeviceMemory textureMemory_;
	VkImageView textureView_;
	VkSampler textureSampler_;
};

