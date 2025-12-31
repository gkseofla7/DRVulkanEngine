#include "Texture.h"
#include "VulkanContext.h"
#include "GlobalData.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>
bool hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}
void SetImageBarrier(VkImageMemoryBarrier& barrier,
    VkPipelineStageFlags& sourceStage,
    VkPipelineStageFlags& destinationStage,
    VkImageLayout oldLayout,
    VkImageLayout newLayout)
{
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
#if USE_GENERAL_LAYOUT
    if (newLayout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR || newLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR)
    {
        barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    }
    if (oldLayout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR || oldLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR)
    {
        barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    }
#endif

    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    // 1. 초기화 (Undefined -> Something)
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        barrier.srcAccessMask = 0;

        if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        // 1. 범용 Attachment 레이아웃 (Color + Depth/Stencil 지원)
        else if (newLayout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR) {
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }
        // 2. 범용 Read Only 레이아웃 (Shader Sampling 등)
        else if (newLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR) {
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // 주로 프래그먼트 셰이더에서 사용
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        }
        else if (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        }
        else if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }
        else if (newLayout == VK_IMAGE_LAYOUT_GENERAL) {
            destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        }
        else {
            throw std::invalid_argument("Unsupported Undefined -> NewLayout transition!");
        }
    }
    // 2. 전송 완료 후 읽기 (Transfer -> Read Only)
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL || newLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR) {
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        }
        else {
            throw std::invalid_argument("Unsupported Transfer -> NewLayout transition!");
        }
    }

    // 3. 렌더링 완료 후 읽기 또는 화면 출력 (Attachment -> Read/Present)
    else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL || oldLayout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR) {
        sourceStage = (oldLayout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR) ?
            (VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT) :
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        if (oldLayout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR) barrier.srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL || newLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR) {
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        }
        else if (newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            barrier.dstAccessMask = 0;
        }
        else if (newLayout == VK_IMAGE_LAYOUT_GENERAL) {
            destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        }
        else {
            throw std::invalid_argument("Unsupported Attachment -> NewLayout transition!");
        }
    }

    // 4. 읽기 전용 상태에서 다시 렌더링 (Read Only -> Attachment)
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL || oldLayout == VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR) {
        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;

        if (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL || newLayout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR) {
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            if (newLayout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR) barrier.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }
        else {
            throw std::invalid_argument("Unsupported ReadOnly -> NewLayout transition!");
        }
    }

    // 5. 범용 레이아웃에서 전환 (General -> Read/Present)
    else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL) {
        sourceStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

        if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        }
        else if (newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            barrier.dstAccessMask = 0;
        }
        else {
            throw std::invalid_argument("Unsupported General -> NewLayout transition!");
        }
    }
    else {
        throw std::invalid_argument("Unsupported layout transition!");
    }
}

Texture::Texture(const VulkanContext* context, const std::string& filepath) {
    this->context = context;
    initialize(filepath);
}

Texture::Texture(const VulkanContext* context, uint32_t width, uint32_t height,
    VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspectFlags)
{
    this->context = context;
    this->format_ = format;

    createImage(width, height, format, VK_IMAGE_TILING_OPTIMAL, usage,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture_, textureMemory_);


    textureView_ = createImageView(texture_, format, aspectFlags);
    if (aspectFlags == VK_IMAGE_ASPECT_COLOR_BIT) {
        // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR
        transitionImageLayout(texture_, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR);
        // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR
        currentLayout_ = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    }
    else if (aspectFlags == VK_IMAGE_ASPECT_DEPTH_BIT) {
        // VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR
        transitionImageLayout(texture_, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        currentLayout_ = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }
    createTextureSampler();

    //imageInfo_.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL -> VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR
    imageInfo_.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR;
    imageInfo_.imageView = textureView_;
    imageInfo_.sampler = textureSampler_;
}

Texture::~Texture() {
    vkDestroyImageView(context->getDevice(), textureView_, nullptr);
    vkDestroyImage(context->getDevice(), texture_, nullptr);
    vkFreeMemory(context->getDevice(), textureMemory_, nullptr);
	vkDestroySampler(context->getDevice(), textureSampler_, nullptr);
}
void Texture::initialize(const std::string& filepath) {
    int texWidth, texHeight, texChannels;
    // stbi_load �Լ��� �̹��� ������ �ҷ��ɴϴ�.
    // STBI_rgb_alpha �÷��״� �̹����� ������ RGBA(4ä��) �������� �ε��մϴ�.
    stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    // CPU���� ���� �����ϰ�(HOST_VISIBLE), ������ ����(TRANSFER_SRC)�� �� ���۸� �����մϴ�.
    createBuffer(imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    // Staging Buffer�� �޸𸮸� �����Ͽ� CPU�� �ȼ� �����͸� �����մϴ�.
    void* data;
    vkMapMemory(context->getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(context->getDevice(), stagingBufferMemory);

    // ���� �ȼ� �����ʹ� ���� CPU �޸𸮿��� �����ص� �˴ϴ�.
    stbi_image_free(pixels);

    format_ = VK_FORMAT_R8G8B8A8_SRGB;
    currentLayout_ = VK_IMAGE_LAYOUT_UNDEFINED;

    // �̹��� ����
    createImage(texWidth, texHeight,
        VK_FORMAT_R8G8B8A8_SRGB, // SRGB ������ ������ �� �ڿ������� ǥ���մϴ�.
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, // ���� ��� + ���̴� ���ø���
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // �ְ��� ������ ���� GPU ���� �޸𸮿� ����
        texture_,
        textureMemory_);

    // (1) UNDEFINED -> TRANSFER_DST_OPTIMAL
    transitionImageLayout(texture_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // (2) Staging Buffer���� ���� �̹����� �ȼ� ������ ����
    copyBufferToImage(stagingBuffer, texture_, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    // (3) TRANSFER_DST_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL 

    // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL -> VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR
    transitionImageLayout(texture_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR);
    // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL -> VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR
    currentLayout_ = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR;

    vkDestroyBuffer(context->getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(context->getDevice(), stagingBufferMemory, nullptr);

    textureView_ = createImageView(texture_, VK_FORMAT_R8G8B8A8_SRGB);

    createTextureSampler();
    // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL -> VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR
    imageInfo_.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR;
    imageInfo_.imageView = textureView_;
    imageInfo_.sampler = textureSampler_;
}


void Texture::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {

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
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; 

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

void Texture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    if (oldLayout == newLayout)
    {
        return;
    }

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (hasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    SetImageBarrier(barrier, sourceStage, destinationStage, currentLayout_, newLayout);

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    endSingleTimeCommands(commandBuffer);
}

void Texture::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    // 2. ������ ����(Region)�� �����մϴ�.
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    endSingleTimeCommands(commandBuffer);
}




VkImageView Texture::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;

    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;

    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(context->getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void Texture::createTextureSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(context->getPhysicalDevice(), &properties);

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(context->getDevice(), &samplerInfo, nullptr, &textureSampler_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void Texture::populateWriteDescriptor(VkWriteDescriptorSet& writeInfo) const
{
    // VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL-> VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR
    imageInfo_.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR;

    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeInfo.descriptorCount = 1;
    writeInfo.pImageInfo = &imageInfo_;
}

void Texture::transitionLayout_Cmd(VkCommandBuffer commandBuffer, VkImageLayout newLayout)
{
    if (currentLayout_ == newLayout) {
        return;
    }

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = currentLayout_;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture_;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (hasStencilComponent(format_)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    SetImageBarrier(barrier, sourceStage, destinationStage, currentLayout_, newLayout);
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage,
        destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    currentLayout_ = newLayout;
}

