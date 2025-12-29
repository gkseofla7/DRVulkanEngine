#include "VulkanSwapChain.h"
#include "VulkanContext.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <algorithm>
#include <iostream>

VulkanSwapChain::~VulkanSwapChain() {
    cleanup();
}

VulkanSwapChain::VulkanSwapChain(VulkanSwapChain&& other) noexcept
    : swapChain(other.swapChain)
    , swapChainImages(std::move(other.swapChainImages))
    , swapChainImageFormat(other.swapChainImageFormat)
    , swapChainExtent(other.swapChainExtent)
    , swapChainImageViews(std::move(other.swapChainImageViews))
    , depthImage(other.depthImage)
    , depthImageMemory(other.depthImageMemory)
    , depthImageView(other.depthImageView)
    , depthFormat(other.depthFormat)
    , context(other.context)
    , window(other.window) {
    
    // 이동된 객체의 핸들들을 무효화
    other.swapChain = VK_NULL_HANDLE;
    other.swapChainImageFormat = VK_FORMAT_UNDEFINED;
    other.swapChainExtent = {0, 0};
    other.depthImage = VK_NULL_HANDLE;
    other.depthImageMemory = VK_NULL_HANDLE;
    other.depthImageView = VK_NULL_HANDLE;
    other.depthFormat = VK_FORMAT_UNDEFINED;
    other.context = nullptr;
    other.window = nullptr;
}

VulkanSwapChain& VulkanSwapChain::operator=(VulkanSwapChain&& other) noexcept {
    if (this != &other) {
        cleanup();
        
        swapChain = other.swapChain;
        swapChainImages = std::move(other.swapChainImages);
        swapChainImageFormat = other.swapChainImageFormat;
        swapChainExtent = other.swapChainExtent;
        swapChainImageViews = std::move(other.swapChainImageViews);
        depthImage = other.depthImage;
        depthImageMemory = other.depthImageMemory;
        depthImageView = other.depthImageView;
        depthFormat = other.depthFormat;
        context = other.context;
        window = other.window;
        
        other.swapChain = VK_NULL_HANDLE;
        other.swapChainImageFormat = VK_FORMAT_UNDEFINED;
        other.swapChainExtent = {0, 0};
        other.depthImage = VK_NULL_HANDLE;
        other.depthImageMemory = VK_NULL_HANDLE;
        other.depthImageView = VK_NULL_HANDLE;
        other.depthFormat = VK_FORMAT_UNDEFINED;
        other.context = nullptr;
        other.window = nullptr;
    }
    return *this;
}

void VulkanSwapChain::initialize(const VulkanContext* vulkanContext, GLFWwindow* glfwWindow) {
    context = vulkanContext;
    window = glfwWindow;
    
    createSwapChain();
    createImageViews();
    createDepthResources(); // Depth Buffer 생성 추가
    
    std::cout << "SwapChain initialized with Dynamic Rendering and Depth Buffer support!" << std::endl;
}

void VulkanSwapChain::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(context->getPhysicalDevice());

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = context->getSurface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const QueueFamilyIndices indices = context->findQueueFamilies(context->getPhysicalDevice());
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(context->getDevice(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(context->getDevice(), swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(context->getDevice(), swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void VulkanSwapChain::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void VulkanSwapChain::createDepthResources() {
    depthFormat = findDepthFormat();

    createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                depthImage, depthImageMemory);

    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    std::cout << "Depth resources created successfully!" << std::endl;
}

void VulkanSwapChain::recreate() {
    // 기존 Depth Buffer 정리
    if (depthImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(context->getDevice(), depthImageView, nullptr);
        depthImageView = VK_NULL_HANDLE;
    }
    if (depthImage != VK_NULL_HANDLE) {
        vkDestroyImage(context->getDevice(), depthImage, nullptr);
        depthImage = VK_NULL_HANDLE;
    }
    if (depthImageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(context->getDevice(), depthImageMemory, nullptr);
        depthImageMemory = VK_NULL_HANDLE;
    }

    // 기존 SwapChain 정리
    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(context->getDevice(), imageView, nullptr);
    }
    swapChainImageViews.clear();

    if (swapChain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(context->getDevice(), swapChain, nullptr);
        swapChain = VK_NULL_HANDLE;
    }

    // 새로운 SwapChain과 Depth Buffer 생성
    createSwapChain();
    createImageViews();
    createDepthResources();

    std::cout << "SwapChain recreated with Dynamic Rendering and Depth Buffer!" << std::endl;
}

void VulkanSwapChain::cleanup() {
    if (!context || !context->getDevice()) {
        return;
    }

    // Depth Buffer 정리
    if (depthImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(context->getDevice(), depthImageView, nullptr);
        depthImageView = VK_NULL_HANDLE;
    }
    if (depthImage != VK_NULL_HANDLE) {
        vkDestroyImage(context->getDevice(), depthImage, nullptr);
        depthImage = VK_NULL_HANDLE;
    }
    if (depthImageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(context->getDevice(), depthImageMemory, nullptr);
        depthImageMemory = VK_NULL_HANDLE;
    }

    // Color Image Views 정리
    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(context->getDevice(), imageView, nullptr);
    }
    swapChainImageViews.clear();

    // SwapChain 정리
    if (swapChain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(context->getDevice(), swapChain, nullptr);
        swapChain = VK_NULL_HANDLE;
    }

    swapChainImages.clear();
}

VkRenderingAttachmentInfo VulkanSwapChain::getColorAttachmentInfo(uint32_t imageIndex) const {
    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = swapChainImageViews[imageIndex];
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    
    // HDR 배경색 - 더 어두운 색상으로 HDR 효과를 더 잘 보이게 함
    colorAttachment.clearValue.color = {{0.01f, 0.01f, 0.02f, 1.0f}}; // 어두운 블루 톤
    
    return colorAttachment;
}

VkRenderingAttachmentInfo VulkanSwapChain::getDepthAttachmentInfo() const {
    VkRenderingAttachmentInfo depthAttachment{};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.imageView = depthImageView;
    // VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.clearValue.depthStencil = {1.0f, 0};
    return depthAttachment;
}

VkRenderingInfo VulkanSwapChain::getRenderingInfo(uint32_t imageIndex) const {
    // thread_local static으로 안전한 참조 보장
    static thread_local VkRenderingAttachmentInfo colorAttachment;
    static thread_local VkRenderingAttachmentInfo depthAttachment;
    
    colorAttachment = getColorAttachmentInfo(imageIndex);
    depthAttachment = getDepthAttachmentInfo();
    
    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea.offset = {0, 0};
    renderingInfo.renderArea.extent = swapChainExtent;
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = &depthAttachment; // Depth Attachment 추가
    renderingInfo.pStencilAttachment = nullptr;
    
    return renderingInfo;
}

VkFormat VulkanSwapChain::findDepthFormat() {
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkFormat VulkanSwapChain::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(context->getPhysicalDevice(), format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

bool VulkanSwapChain::hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VulkanSwapChain::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, 
                                  VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
                                  VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(context->getDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(context->getDevice(), image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = context->findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(context->getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(context->getDevice(), image, imageMemory, 0);
}

VkImageView VulkanSwapChain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(context->getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view!");
    }

    return imageView;
}

SwapChainSupportDetails VulkanSwapChain::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, context->getSurface(), &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, context->getSurface(), &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, context->getSurface(), &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, context->getSurface(), &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, context->getSurface(), &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR VulkanSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    // HDR10 지원을 위한 포맷 우선순위
    
    //// 1. HDR10 - Rec. 2020 color space with PQ transfer function
    //for (const auto& availableFormat : availableFormats) {
    //    if (availableFormat.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 && 
    //        availableFormat.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT) {
    //        std::cout << "HDR10 format selected: VK_FORMAT_A2B10G10R10_UNORM_PACK32 with HDR10_ST2084" << std::endl;
    //        return availableFormat;
    //    }
    //}
    //
    //// 2. Extended sRGB for wider color gamut
    //for (const auto& availableFormat : availableFormats) {
    //    if (availableFormat.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 && 
    //        availableFormat.colorSpace == VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT) {
    //        std::cout << "Extended sRGB format selected: VK_FORMAT_A2B10G10R10_UNORM_PACK32" << std::endl;
    //        return availableFormat;
    //    }
    //}
    //
    //// 3. 16-bit float format for HDR
    //for (const auto& availableFormat : availableFormats) {
    //    if (availableFormat.format == VK_FORMAT_R16G16B16A16_SFLOAT && 
    //        availableFormat.colorSpace == VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT) {
    //        std::cout << "HDR float16 format selected: VK_FORMAT_R16G16B16A16_SFLOAT" << std::endl;
    //        return availableFormat;
    //    }
    //}
    //
    //// 4. 10-bit format with standard sRGB (wider color gamut)
    //for (const auto& availableFormat : availableFormats) {
    //    if (availableFormat.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 && 
    //        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
    //        std::cout << "10-bit sRGB format selected: VK_FORMAT_A2B10G10R10_UNORM_PACK32" << std::endl;
    //        return availableFormat;
    //    }
    //}
    
    // 5. Fallback to standard sRGB
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && 
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            std::cout << "Standard sRGB format selected: VK_FORMAT_B8G8R8A8_SRGB" << std::endl;
            return availableFormat;
        }
    }
    
    std::cout << "Default format selected: " << availableFormats[0].format << std::endl;
    return availableFormats[0];
}

VkPresentModeKHR VulkanSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}