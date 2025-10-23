#include "Texture.h"
#include "VulkanContext.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>

Texture::Texture(const VulkanContext* context, const std::string& filepath) {
    // Resource 클래스의 context 멤버 변수 등을 초기화
    this->context = context;

    // 파일 경로를 받아 초기화 함수 호출
    initialize(filepath);
}

Texture::~Texture() {
    // 생성된 순서의 역순으로 리소스를 정리합니다.
    vkDestroyImageView(context->getDevice(), textureView_, nullptr);
    vkDestroyImage(context->getDevice(), texture_, nullptr);
    vkFreeMemory(context->getDevice(), textureMemory_, nullptr);
	vkDestroySampler(context->getDevice(), textureSampler_, nullptr);
    // Sampler가 있다면 vkDestroySampler(...)도 호출
}
void Texture::initialize(const std::string& filepath) {
    int texWidth, texHeight, texChannels;
    // stbi_load 함수로 이미지 파일을 불러옵니다.
    // STBI_rgb_alpha 플래그는 이미지를 강제로 RGBA(4채널) 형식으로 로드합니다.
    stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    // CPU에서 접근 가능하고(HOST_VISIBLE), 복사의 원본(TRANSFER_SRC)이 될 버퍼를 생성합니다.
    createBuffer(imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    // Staging Buffer의 메모리를 매핑하여 CPU의 픽셀 데이터를 복사합니다.
    void* data;
    vkMapMemory(context->getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(context->getDevice(), stagingBufferMemory);

    // 원본 픽셀 데이터는 이제 CPU 메모리에서 해제해도 됩니다.
    stbi_image_free(pixels);


    // 이미지 생성
    createImage(texWidth, texHeight,
        VK_FORMAT_R8G8B8A8_SRGB, // SRGB 포맷은 색상을 더 자연스럽게 표현합니다.
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, // 복사 대상 + 셰이더 샘플링용
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // 최고의 성능을 위해 GPU 전용 메모리에 생성
        texture_,
        textureMemory_);

    // 레이아웃 변경 및 데이터 복사
    // (1) UNDEFINED -> TRANSFER_DST_OPTIMAL (복사 받기 좋은 레이아웃으로 변경)
    transitionImageLayout(texture_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // (2) Staging Buffer에서 최종 이미지로 픽셀 데이터 복사
    copyBufferToImage(stagingBuffer, texture_, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    // (3) TRANSFER_DST_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL (셰이더가 읽기 좋은 레이아웃으로 변경)
    transitionImageLayout(texture_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // 복사가 끝났으므로 Staging Buffer는 파괴합니다.
    vkDestroyBuffer(context->getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(context->getDevice(), stagingBufferMemory, nullptr);

    // 이미지 뷰 생성
    // 셰이더는 VkImage가 아닌 VkImageView를 통해 이미지에 접근합니다.
    textureView_ = createImageView(texture_, VK_FORMAT_R8G8B8A8_SRGB);

    createTextureSampler();
}


void Texture::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    // 1. 이미지 생성 정보(CreateInfo) 정의
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1; // 2D 텍스처이므로 depth는 1
    imageInfo.mipLevels = 1; // 지금은 밉맵을 사용하지 않음
    imageInfo.arrayLayers = 1; // 배열 텍스처가 아님
    imageInfo.format = format; // 이미지의 픽셀 포맷 (예: RGBA)
    imageInfo.tiling = tiling; // 텍셀 배치 방식 (Optimal이 GPU에 최적)
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // 초기 레이아웃은 신경 쓰지 않음
    imageInfo.usage = usage; // 이미지의 용도 (예: 텍스처 샘플링, 복사 대상)
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // 멀티샘플링 사용 안 함

    // 2. 이미지 핸들 생성
    if (vkCreateImage(context->getDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    // 3. 이미지에 필요한 메모리 요구사항 쿼리
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(context->getDevice(), image, &memRequirements);

    // 4. 메모리 할당 정보(AllocateInfo) 정의
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = context->findMemoryType(memRequirements.memoryTypeBits, properties);

    // 5. GPU 메모리 할당
    if (vkAllocateMemory(context->getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    // 6. 할당된 메모리를 이미지에 바인딩(연결)
    vkBindImageMemory(context->getDevice(), image, imageMemory, 0);
}


void Texture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    // 1. 일회성 작업을 위한 커맨드 버퍼를 시작합니다.
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    // 2. 이미지 메모리 장벽(Image Memory Barrier)을 설정합니다.
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout; // 이전 레이아웃
    barrier.newLayout = newLayout; // 새로운 레이아웃
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

    // 3. 이전/이후 레이아웃에 따라 어떤 파이프라인 단계에서
    //    장벽이 발생해야 하는지, 접근 권한은 어떻게 설정할지 결정합니다.
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

    // 4. 커맨드 버퍼에 파이프라인 장벽 명령을 기록합니다.
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    // 5. 커맨드 버퍼 기록을 마치고 GPU에 제출하여 실행합니다.
    endSingleTimeCommands(commandBuffer);
}

void Texture::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    // 1. 일회성 작업을 위한 커맨드 버퍼를 시작합니다.
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    // 2. 복사할 영역(Region)을 정의합니다.
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

    // 3. 커맨드 버퍼에 버퍼-이미지 복사 명령을 기록합니다.
    //    이미지의 레이아웃은 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL 상태여야 합니다.
    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    // 4. 커맨드 버퍼 기록을 마치고 GPU에 제출하여 실행합니다.
    endSingleTimeCommands(commandBuffer);
}




VkImageView Texture::createImageView(VkImage image, VkFormat format) {
    // 1. 이미지 뷰 생성 정보(CreateInfo)를 정의합니다.
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image; // 뷰를 생성할 대상 이미지

    // 2. 이미지의 타입을 지정합니다. (2D, 3D, Cubemap 등)
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format; // 뷰가 이미지를 해석할 픽셀 포맷

    // 3. 컴포넌트 매핑(swizzling)을 기본값으로 설정합니다.
    // (예: R 채널을 G 채널로 보이게 하는 등의 작업을 하지 않음)
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    // 4. 이미지의 어떤 부분을 뷰가 다룰지 정의합니다. (Subresource Range)
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // 컬러 데이터를 다룸
    viewInfo.subresourceRange.baseMipLevel = 0; // 밉맵 레벨 0부터
    viewInfo.subresourceRange.levelCount = 1;   // 1개의 밉맵 레벨을 다룸
    viewInfo.subresourceRange.baseArrayLayer = 0; // 배열의 첫 번째 레이어부터
    viewInfo.subresourceRange.layerCount = 1;   // 1개의 레이어를 다룸

    VkImageView imageView;
    if (vkCreateImageView(context->getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void Texture::createTextureSampler() {
    // 1. 샘플러 생성 정보(CreateInfo)를 정의합니다.
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    // 2. 필터링(Filtering) 설정: 텍스처를 확대(mag)하거나 축소(min)할 때
    //    픽셀 색상을 어떻게 보간할지 결정합니다.
    //    - VK_FILTER_LINEAR: 주변 픽셀과 선형 보간하여 부드럽게 표현합니다. (추천)
    //    - VK_FILTER_NEAREST: 가장 가까운 픽셀 색상을 그대로 사용하여 픽셀 아트처럼 보이게 합니다.
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    // 3. 주소 지정 모드(Address Mode): UV 좌표가 [0, 1] 범위를 벗어날 때
    //    텍스처를 어떻게 처리할지 결정합니다.
    //    - VK_SAMPLER_ADDRESS_MODE_REPEAT: 텍스처를 바둑판식으로 반복합니다. (일반적)
    //    - VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: 거울에 비친 것처럼 반복합니다.
    //    - VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: 가장자리 픽셀 색상을 계속 사용합니다.
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    // 4. 비등방성 필터링(Anisotropic Filtering) 활성화  anisotropia
    //    비스듬한 각도에서 텍스처를 볼 때 흐릿해지는 현상을 줄여 퀄리티를 크게 향상시킵니다.
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(context->getPhysicalDevice(), &properties);

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // GPU가 지원하는 최대값 사용

    // 5. 기타 설정
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // Clamp 모드일 때의 테두리 색상
    samplerInfo.unnormalizedCoordinates = VK_FALSE; // UV 좌표를 [0, 1]로 정규화하여 사용
    samplerInfo.compareEnable = VK_FALSE; // 그림자 맵(PCF) 같은 특수 용도가 아님
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    // 6. 밉맵(Mipmap) 관련 설정
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f; // 지금은 밉맵을 사용하지 않으므로 기본값 설정

    if (vkCreateSampler(context->getDevice(), &samplerInfo, nullptr, &textureSampler_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}