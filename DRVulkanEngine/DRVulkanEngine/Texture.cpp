#include "Texture.h"
#include "VulkanContext.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>

Texture::Texture(const VulkanContext* context, const std::string& filepath) {
    // Resource Ŭ������ context ��� ���� ���� �ʱ�ȭ
    this->context = context;

    // ���� ��θ� �޾� �ʱ�ȭ �Լ� ȣ��
    initialize(filepath);
}

Texture::~Texture() {
    // ������ ������ �������� ���ҽ��� �����մϴ�.
    vkDestroyImageView(context->getDevice(), textureView_, nullptr);
    vkDestroyImage(context->getDevice(), texture_, nullptr);
    vkFreeMemory(context->getDevice(), textureMemory_, nullptr);
	vkDestroySampler(context->getDevice(), textureSampler_, nullptr);
    // Sampler�� �ִٸ� vkDestroySampler(...)�� ȣ��
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


    // �̹��� ����
    createImage(texWidth, texHeight,
        VK_FORMAT_R8G8B8A8_SRGB, // SRGB ������ ������ �� �ڿ������� ǥ���մϴ�.
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, // ���� ��� + ���̴� ���ø���
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // �ְ��� ������ ���� GPU ���� �޸𸮿� ����
        texture_,
        textureMemory_);

    // ���̾ƿ� ���� �� ������ ����
    // (1) UNDEFINED -> TRANSFER_DST_OPTIMAL (���� �ޱ� ���� ���̾ƿ����� ����)
    transitionImageLayout(texture_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // (2) Staging Buffer���� ���� �̹����� �ȼ� ������ ����
    copyBufferToImage(stagingBuffer, texture_, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    // (3) TRANSFER_DST_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL (���̴��� �б� ���� ���̾ƿ����� ����)
    transitionImageLayout(texture_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // ���簡 �������Ƿ� Staging Buffer�� �ı��մϴ�.
    vkDestroyBuffer(context->getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(context->getDevice(), stagingBufferMemory, nullptr);

    // �̹��� �� ����
    // ���̴��� VkImage�� �ƴ� VkImageView�� ���� �̹����� �����մϴ�.
    textureView_ = createImageView(texture_, VK_FORMAT_R8G8B8A8_SRGB);

    createTextureSampler();
}


void Texture::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    // 1. �̹��� ���� ����(CreateInfo) ����
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1; // 2D �ؽ�ó�̹Ƿ� depth�� 1
    imageInfo.mipLevels = 1; // ������ �Ӹ��� ������� ����
    imageInfo.arrayLayers = 1; // �迭 �ؽ�ó�� �ƴ�
    imageInfo.format = format; // �̹����� �ȼ� ���� (��: RGBA)
    imageInfo.tiling = tiling; // �ؼ� ��ġ ��� (Optimal�� GPU�� ����)
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // �ʱ� ���̾ƿ��� �Ű� ���� ����
    imageInfo.usage = usage; // �̹����� �뵵 (��: �ؽ�ó ���ø�, ���� ���)
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // ��Ƽ���ø� ��� �� ��

    // 2. �̹��� �ڵ� ����
    if (vkCreateImage(context->getDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    // 3. �̹����� �ʿ��� �޸� �䱸���� ����
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(context->getDevice(), image, &memRequirements);

    // 4. �޸� �Ҵ� ����(AllocateInfo) ����
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = context->findMemoryType(memRequirements.memoryTypeBits, properties);

    // 5. GPU �޸� �Ҵ�
    if (vkAllocateMemory(context->getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    // 6. �Ҵ�� �޸𸮸� �̹����� ���ε�(����)
    vkBindImageMemory(context->getDevice(), image, imageMemory, 0);
}


void Texture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    // 1. ��ȸ�� �۾��� ���� Ŀ�ǵ� ���۸� �����մϴ�.
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    // 2. �̹��� �޸� �庮(Image Memory Barrier)�� �����մϴ�.
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout; // ���� ���̾ƿ�
    barrier.newLayout = newLayout; // ���ο� ���̾ƿ�
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    // 3. ����/���� ���̾ƿ��� ���� � ���������� �ܰ迡��
    //    �庮�� �߻��ؾ� �ϴ���, ���� ������ ��� �������� �����մϴ�.
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    // 4. Ŀ�ǵ� ���ۿ� ���������� �庮 ����� ����մϴ�.
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    // 5. Ŀ�ǵ� ���� ����� ��ġ�� GPU�� �����Ͽ� �����մϴ�.
    endSingleTimeCommands(commandBuffer);
}

void Texture::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    // 1. ��ȸ�� �۾��� ���� Ŀ�ǵ� ���۸� �����մϴ�.
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

    // 3. Ŀ�ǵ� ���ۿ� ����-�̹��� ���� ����� ����մϴ�.
    //    �̹����� ���̾ƿ��� VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ���¿��� �մϴ�.
    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    // 4. Ŀ�ǵ� ���� ����� ��ġ�� GPU�� �����Ͽ� �����մϴ�.
    endSingleTimeCommands(commandBuffer);
}




VkImageView Texture::createImageView(VkImage image, VkFormat format) {
    // 1. �̹��� �� ���� ����(CreateInfo)�� �����մϴ�.
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image; // �並 ������ ��� �̹���

    // 2. �̹����� Ÿ���� �����մϴ�. (2D, 3D, Cubemap ��)
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format; // �䰡 �̹����� �ؼ��� �ȼ� ����

    // 3. ������Ʈ ����(swizzling)�� �⺻������ �����մϴ�.
    // (��: R ä���� G ä�η� ���̰� �ϴ� ���� �۾��� ���� ����)
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    // 4. �̹����� � �κ��� �䰡 �ٷ��� �����մϴ�. (Subresource Range)
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // �÷� �����͸� �ٷ�
    viewInfo.subresourceRange.baseMipLevel = 0; // �Ӹ� ���� 0����
    viewInfo.subresourceRange.levelCount = 1;   // 1���� �Ӹ� ������ �ٷ�
    viewInfo.subresourceRange.baseArrayLayer = 0; // �迭�� ù ��° ���̾����
    viewInfo.subresourceRange.layerCount = 1;   // 1���� ���̾ �ٷ�

    VkImageView imageView;
    if (vkCreateImageView(context->getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void Texture::createTextureSampler() {
    // 1. ���÷� ���� ����(CreateInfo)�� �����մϴ�.
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    // 2. ���͸�(Filtering) ����: �ؽ�ó�� Ȯ��(mag)�ϰų� ���(min)�� ��
    //    �ȼ� ������ ��� �������� �����մϴ�.
    //    - VK_FILTER_LINEAR: �ֺ� �ȼ��� ���� �����Ͽ� �ε巴�� ǥ���մϴ�. (��õ)
    //    - VK_FILTER_NEAREST: ���� ����� �ȼ� ������ �״�� ����Ͽ� �ȼ� ��Ʈó�� ���̰� �մϴ�.
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    // 3. �ּ� ���� ���(Address Mode): UV ��ǥ�� [0, 1] ������ ��� ��
    //    �ؽ�ó�� ��� ó������ �����մϴ�.
    //    - VK_SAMPLER_ADDRESS_MODE_REPEAT: �ؽ�ó�� �ٵ��ǽ����� �ݺ��մϴ�. (�Ϲ���)
    //    - VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: �ſ￡ ��ģ ��ó�� �ݺ��մϴ�.
    //    - VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: �����ڸ� �ȼ� ������ ��� ����մϴ�.
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    // 4. ���漺 ���͸�(Anisotropic Filtering) Ȱ��ȭ  anisotropia
    //    �񽺵��� �������� �ؽ�ó�� �� �� �帴������ ������ �ٿ� ����Ƽ�� ũ�� ����ŵ�ϴ�.
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(context->getPhysicalDevice(), &properties);

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // GPU�� �����ϴ� �ִ밪 ���

    // 5. ��Ÿ ����
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // Clamp ����� ���� �׵θ� ����
    samplerInfo.unnormalizedCoordinates = VK_FALSE; // UV ��ǥ�� [0, 1]�� ����ȭ�Ͽ� ���
    samplerInfo.compareEnable = VK_FALSE; // �׸��� ��(PCF) ���� Ư�� �뵵�� �ƴ�
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    // 6. �Ӹ�(Mipmap) ���� ����
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f; // ������ �Ӹ��� ������� �����Ƿ� �⺻�� ����

    if (vkCreateSampler(context->getDevice(), &samplerInfo, nullptr, &textureSampler_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}