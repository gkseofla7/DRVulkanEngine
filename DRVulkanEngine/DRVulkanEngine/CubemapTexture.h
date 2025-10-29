#pragma once
#include "Resource.h"
#include <string>
#include <array>

class CubemapTexture : public Resource
{
public:
    // EXR 환경맵으로부터 큐브맵 생성
    CubemapTexture(const class VulkanContext* context, const std::string& hdrFilePath, uint32_t cubemapSize = 512);
    
    // 6개의 개별 이미지 파일로부터 큐브맵 생성
    CubemapTexture(const class VulkanContext* context, const std::array<std::string, 6>& cubeFaces);
    
    ~CubemapTexture();

    // Getter 메서드
    VkImageView getImageView() const { return cubemapView_; }
    VkSampler getSampler() const { return cubemapSampler_; }
    VkDescriptorImageInfo getImageInfo() const { return imageInfo_; }
    virtual void populateWriteDescriptor(VkWriteDescriptorSet& writeInfo) const override;

private:
    // EXR 파일로부터 큐브맵 생성
    void createFromHDR(const std::string& hdrFilePath, uint32_t cubemapSize);
    
    // 6개 개별 파일로부터 큐브맵 생성
    void createFromSixImages(const std::array<std::string, 6>& cubeFaces);
    
    // HDR 이미지를 큐브맵으로 변환하는 함수
    void convertEquirectangularToCubemap(float* hdrData, int hdrWidth, int hdrHeight, uint32_t cubemapSize);
    
    // 큐브맵 이미지 생성
    void createCubemapImage(uint32_t width, uint32_t height, VkFormat format, 
                           VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
    
    // 큐브맵 이미지 뷰 생성
    void createCubemapImageView(VkFormat format);
    
    // 큐브맵 샘플러 생성
    void createCubemapSampler();
    
    // 이미지 레이아웃 전환
    void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
    
    // 버퍼에서 큐브맵으로 데이터 복사
    void copyBufferToCubemap(VkBuffer buffer, uint32_t width, uint32_t height, bool isHDR = false);
    
    // 구면 좌표를 큐브 좌표로 변환하는 유틸리티 함수들
    struct CubeCoord {
        int face;
        float u, v;
    };
    CubeCoord sphericalToCube(float theta, float phi) const;
    
    // 큐브 페이스별 방향 벡터 계산
    void getCubeFaceDirection(int face, float u, float v, float& x, float& y, float& z) const;

private:
    VkImage cubemapImage_;
    VkDeviceMemory cubemapMemory_;
    VkImageView cubemapView_;
    VkSampler cubemapSampler_;
    
    VkDescriptorImageInfo imageInfo_;
    uint32_t cubemapSize_;
    
    // 큐브맵 페이스 순서: +X, -X, +Y, -Y, +Z, -Z
    static constexpr int CUBE_FACES = 6;
    static const std::array<const char*, CUBE_FACES> FACE_NAMES;
};