#pragma once
#include <vulkan/vulkan.h>
#include <memory>

class VulkanContext;
class Texture;

class RenderTarget
{
public:
    RenderTarget() = default;

    // 컬러, 깊이 이미지를 생성하고 관리
    void initialize(VulkanContext* ctx, VkExtent2D extent, VkFormat colorFormat, VkFormat depthFormat);
    void cleanup();

    // Getter
    VkImageView getColorView() const;
    VkImageView getDepthView() const;
    Texture* getColorTexture() const { return colorTexture_.get(); }

    VkExtent2D getExtent() const { return extent_; }
    VkFormat getColorFormat() const { return colorFormat_; }
    VkFormat getDepthFormat() const { return depthFormat_; }

    VkRenderingAttachmentInfo getColorAttachmentInfo() const;
    VkRenderingAttachmentInfo getDepthAttachmentInfo() const;
    VkRenderingInfo getRenderingInfo() const;

private:
    std::unique_ptr<Texture> colorTexture_;
    std::unique_ptr<Texture> depthTexture_;

    VkExtent2D extent_;

    VkFormat colorFormat_;
    VkFormat depthFormat_;
};