#include "RenderTarget.h"
#include "Texture.h"
#include "VulkanContext.h"

void RenderTarget::initialize(VulkanContext* ctx, VkExtent2D extent, VkFormat colorFormat, VkFormat depthFormat)
{
    extent_ = extent;
    colorFormat_ = colorFormat;
    depthFormat_ = depthFormat;

    VkImageUsageFlags colorUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    colorTexture_ = std::make_unique<Texture>(ctx, extent.width, extent.height,
        colorFormat, colorUsage, VK_IMAGE_ASPECT_COLOR_BIT);

    VkImageUsageFlags depthUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    depthTexture_ = std::make_unique<Texture>(ctx, extent.width, extent.height,
        depthFormat, depthUsage, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void RenderTarget::cleanup()
{
    colorTexture_.reset();
    depthTexture_.reset();
}

VkImageView RenderTarget::getColorView() const
{
    return colorTexture_ ? colorTexture_->getImageView() : VK_NULL_HANDLE;
}

VkImageView RenderTarget::getDepthView() const
{
    return depthTexture_ ? depthTexture_->getImageView() : VK_NULL_HANDLE;
}

VkRenderingAttachmentInfo RenderTarget::getColorAttachmentInfo() const
{
    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = getColorView();
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

    return colorAttachment;
}

VkRenderingAttachmentInfo RenderTarget::getDepthAttachmentInfo() const
{
    VkRenderingAttachmentInfo depthAttachment{};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.imageView = getDepthView();
    // VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.clearValue.depthStencil = { 1.0f, 0 };

    return depthAttachment;
}

VkRenderingInfo RenderTarget::getRenderingInfo() const
{
    static thread_local VkRenderingAttachmentInfo colorAttachment;
    static thread_local VkRenderingAttachmentInfo depthAttachment;

    colorAttachment = getColorAttachmentInfo();
    depthAttachment = getDepthAttachmentInfo();

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea.offset = { 0, 0 };
    renderingInfo.renderArea.extent = extent_;
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = &depthAttachment;
    renderingInfo.pStencilAttachment = nullptr;

    return renderingInfo;
}