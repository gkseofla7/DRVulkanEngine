#include "CubemapTexture.h"
#include "VulkanContext.h"
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <vector>

// STB 이미지 로더 (HDR 지원) - IMPLEMENTATION은 제거
#include <stb_image.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const std::array<const char*, CubemapTexture::CUBE_FACES> CubemapTexture::FACE_NAMES = {
    "Right", "Left", "Top", "Bottom", "Front", "Back"
};

CubemapTexture::CubemapTexture(const VulkanContext* context, const std::string& hdrFilePath, uint32_t cubemapSize)
    : cubemapSize_(cubemapSize)
{
    this->context = context;
    createFromHDR(hdrFilePath, cubemapSize);
}

CubemapTexture::CubemapTexture(const VulkanContext* context, const std::array<std::string, 6>& cubeFaces)
    : cubemapSize_(512) // 기본 크기
{
    this->context = context;
    createFromSixImages(cubeFaces);
}

CubemapTexture::~CubemapTexture() {
    if (context && context->getDevice()) {
        vkDestroyImageView(context->getDevice(), cubemapView_, nullptr);
        vkDestroyImage(context->getDevice(), cubemapImage_, nullptr);
        vkFreeMemory(context->getDevice(), cubemapMemory_, nullptr);
        vkDestroySampler(context->getDevice(), cubemapSampler_, nullptr);
    }
}

void CubemapTexture::createFromHDR(const std::string& hdrFilePath, uint32_t cubemapSize) {
    // HDR 이미지 로드 (32-bit float)
    int hdrWidth, hdrHeight, hdrChannels;
    float* hdrData = stbi_loadf(hdrFilePath.c_str(), &hdrWidth, &hdrHeight, &hdrChannels, 4);
    
    if (!hdrData) {
        throw std::runtime_error("Failed to load HDR image: " + hdrFilePath);
    }

    try {
        // 큐브맵 이미지 생성
        createCubemapImage(cubemapSize, cubemapSize, VK_FORMAT_R32G32B32A32_SFLOAT,
                          VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        // HDR을 큐브맵으로 변환
        convertEquirectangularToCubemap(hdrData, hdrWidth, hdrHeight, cubemapSize);

        // 이미지 뷰와 샘플러 생성
        createCubemapImageView(VK_FORMAT_R32G32B32A32_SFLOAT);
        createCubemapSampler();

        // Descriptor 정보 설정
        imageInfo_.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo_.imageView = cubemapView_;
        imageInfo_.sampler = cubemapSampler_;

    } catch (...) {
        stbi_image_free(hdrData);
        throw;
    }

    stbi_image_free(hdrData);
}

void CubemapTexture::createFromSixImages(const std::array<std::string, 6>& cubeFaces) {
    // 첫 번째 이미지로 크기 결정
    int width, height, channels;
    unsigned char* testImage = stbi_load(cubeFaces[0].c_str(), &width, &height, &channels, 4);
    if (!testImage) {
        throw std::runtime_error("Failed to load cube face: " + cubeFaces[0]);
    }
    stbi_image_free(testImage);
    
    cubemapSize_ = width; // 정사각형이라고 가정

    // 큐브맵 이미지 생성
    createCubemapImage(cubemapSize_, cubemapSize_, VK_FORMAT_R8G8B8A8_SRGB,
                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // 스테이징 버퍼 생성
    VkDeviceSize imageSize = cubemapSize_ * cubemapSize_ * 4; // RGBA
    VkDeviceSize totalSize = imageSize * CUBE_FACES;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    
    createBuffer(totalSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer,
                stagingBufferMemory);

    // 각 큐브 페이스 로드 및 스테이징 버퍼에 복사
    void* data;
    vkMapMemory(context->getDevice(), stagingBufferMemory, 0, totalSize, 0, &data);

    for (int face = 0; face < CUBE_FACES; ++face) {
        int imgWidth, imgHeight, imgChannels;
        unsigned char* faceData = stbi_load(cubeFaces[face].c_str(), &imgWidth, &imgHeight, &imgChannels, 4);
        if (!faceData) {
            vkUnmapMemory(context->getDevice(), stagingBufferMemory);
            throw std::runtime_error("Failed to load cube face: " + cubeFaces[face]);
        }

        memcpy(static_cast<char*>(data) + face * imageSize, faceData, imageSize);
        stbi_image_free(faceData);
    }

    vkUnmapMemory(context->getDevice(), stagingBufferMemory);

    // 이미지 레이아웃 전환 및 데이터 복사
    transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToCubemap(stagingBuffer, cubemapSize_, cubemapSize_, false); // LDR 이미지
    transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // 스테이징 버퍼 정리
    vkDestroyBuffer(context->getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(context->getDevice(), stagingBufferMemory, nullptr);

    // 이미지 뷰와 샘플러 생성
    createCubemapImageView(VK_FORMAT_R8G8B8A8_SRGB);
    createCubemapSampler();

    // Descriptor 정보 설정
    imageInfo_.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo_.imageView = cubemapView_;
    imageInfo_.sampler = cubemapSampler_;
}

void CubemapTexture::convertEquirectangularToCubemap(float* hdrData, int hdrWidth, int hdrHeight, uint32_t cubemapSize) {
    // 큐브맵 데이터를 위한 메모리 할당
    const size_t faceSize = cubemapSize * cubemapSize * 4 * sizeof(float); // RGBA32F
    const size_t totalSize = faceSize * CUBE_FACES;
    
    std::vector<float> cubemapData(totalSize / sizeof(float));
    
    // 각 큐브 페이스 생성
    for (int face = 0; face < CUBE_FACES; ++face) {
        float* faceData = &cubemapData[face * cubemapSize * cubemapSize * 4];
        
        for (uint32_t y = 0; y < cubemapSize; ++y) {
            for (uint32_t x = 0; x < cubemapSize; ++x) {
                // UV 좌표 계산 (0-1 범위를 -1 to +1로 변환)
                float u = (2.0f * (x + 0.5f) / cubemapSize) - 1.0f;
                float v = (2.0f * (y + 0.5f) / cubemapSize) - 1.0f;
                
                // 큐브 페이스에서의 3D 방향 벡터 계산
                float dirX, dirY, dirZ;
                getCubeFaceDirection(face, u, v, dirX, dirY, dirZ);
                
                // 구면 좌표로 변환
                float theta = atan2f(dirZ, dirX);
                float phi = asinf(dirY);
                
                // 구면 좌표를 equirectangular UV로 변환
                float hdrU = (theta + (float)M_PI) / (2.0f * (float)M_PI);
                float hdrV = (phi + (float)M_PI / 2.0f) / (float)M_PI;
                
                // HDR 이미지에서 픽셀 샘플링 (bilinear interpolation)
                float hdrX = hdrU * (hdrWidth - 1);
                float hdrY = hdrV * (hdrHeight - 1);
                
                int x0 = (int)hdrX;
                int y0 = (int)hdrY;
                int x1 = std::min(x0 + 1, hdrWidth - 1);
                int y1 = std::min(y0 + 1, hdrHeight - 1);
                
                float fx = hdrX - x0;
                float fy = hdrY - y0;
                
                // 4개 인접 픽셀에서 보간
                for (int c = 0; c < 4; ++c) {
                    float p00 = hdrData[(y0 * hdrWidth + x0) * 4 + c];
                    float p01 = hdrData[(y0 * hdrWidth + x1) * 4 + c];
                    float p10 = hdrData[(y1 * hdrWidth + x0) * 4 + c];
                    float p11 = hdrData[(y1 * hdrWidth + x1) * 4 + c];
                    
                    float p0 = p00 * (1 - fx) + p01 * fx;
                    float p1 = p10 * (1 - fx) + p11 * fx;
                    float result = p0 * (1 - fy) + p1 * fy;
                    
                    faceData[(y * cubemapSize + x) * 4 + c] = result;
                }
            }
        }
    }
    
    // 스테이징 버퍼 생성 및 데이터 복사
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    
    createBuffer(totalSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer,
                stagingBufferMemory);

    void* data;
    vkMapMemory(context->getDevice(), stagingBufferMemory, 0, totalSize, 0, &data);
    memcpy(data, cubemapData.data(), totalSize);
    vkUnmapMemory(context->getDevice(), stagingBufferMemory);

    // 이미지 레이아웃 전환 및 데이터 복사
    transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToCubemap(stagingBuffer, cubemapSize, cubemapSize, true); // HDR 이미지
    transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // 스테이징 버퍼 정리
    vkDestroyBuffer(context->getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(context->getDevice(), stagingBufferMemory, nullptr);
}

void CubemapTexture::getCubeFaceDirection(int face, float u, float v, float& x, float& y, float& z) const {
    switch (face) {
        case 0: // +X (Right)
            x = 1.0f;
            y = -v;
            z = -u;
            break;
        case 1: // -X (Left)
            x = -1.0f;
            y = -v;
            z = u;
            break;
        case 2: // +Y (Top)
            x = u;
            y = 1.0f;
            z = v;
            break;
        case 3: // -Y (Bottom)
            x = u;
            y = -1.0f;
            z = -v;
            break;
        case 4: // +Z (Front)
            x = u;
            y = -v;
            z = 1.0f;
            break;
        case 5: // -Z (Back)
            x = -u;
            y = -v;
            z = -1.0f;
            break;
    }
    
    // 정규화
    float length = sqrtf(x * x + y * y + z * z);
    x /= length;
    y /= length;
    z /= length;
}

void CubemapTexture::createCubemapImage(uint32_t width, uint32_t height, VkFormat format, 
                                       VkImageUsageFlags usage, VkMemoryPropertyFlags properties) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT; // 큐브맵 플래그 중요!
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 6; // 큐브맵은 6개 레이어
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if (vkCreateImage(context->getDevice(), &imageInfo, nullptr, &cubemapImage_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create cubemap image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(context->getDevice(), cubemapImage_, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = context->findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(context->getDevice(), &allocInfo, nullptr, &cubemapMemory_) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate cubemap memory!");
    }

    vkBindImageMemory(context->getDevice(), cubemapImage_, cubemapMemory_, 0);
}

void CubemapTexture::createCubemapImageView(VkFormat format) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = cubemapImage_;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE; // 큐브맵 뷰 타입
    viewInfo.format = format;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 6; // 6개 레이어 모두

    if (vkCreateImageView(context->getDevice(), &viewInfo, nullptr, &cubemapView_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create cubemap image view!");
    }
}

void CubemapTexture::createCubemapSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // 큐브맵은 clamp 권장
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

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

    if (vkCreateSampler(context->getDevice(), &samplerInfo, nullptr, &cubemapSampler_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create cubemap sampler!");
    }
}

void CubemapTexture::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = cubemapImage_;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 6; // 6개 레이어 모두

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    endSingleTimeCommands(commandBuffer);
}

void CubemapTexture::copyBufferToCubemap(VkBuffer buffer, uint32_t width, uint32_t height, bool isHDR) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    std::vector<VkBufferImageCopy> regions(CUBE_FACES);
    VkDeviceSize faceSize = width * height * 4 * (isHDR ? sizeof(float) : sizeof(unsigned char));

    for (int face = 0; face < CUBE_FACES; ++face) {
        regions[face].bufferOffset = face * faceSize;
        regions[face].bufferRowLength = 0;
        regions[face].bufferImageHeight = 0;
        regions[face].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        regions[face].imageSubresource.mipLevel = 0;
        regions[face].imageSubresource.baseArrayLayer = face;
        regions[face].imageSubresource.layerCount = 1;
        regions[face].imageOffset = {0, 0, 0};
        regions[face].imageExtent = {width, height, 1};
    }

    vkCmdCopyBufferToImage(commandBuffer, buffer, cubemapImage_, 
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                          CUBE_FACES, regions.data());

    endSingleTimeCommands(commandBuffer);
}

void CubemapTexture::populateWriteDescriptor(VkWriteDescriptorSet& writeInfo) const {
    writeInfo.descriptorCount = 1;
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeInfo.pImageInfo = &imageInfo_;
}