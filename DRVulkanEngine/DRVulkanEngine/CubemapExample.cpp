// CubemapTexture 사용 예시

#include "CubemapTexture.h"
#include "VulkanContext.h"
#include <memory>
#include <iostream>
#include <array>

// 1. HDR/EXR 파일로부터 큐브맵 생성
void createEnvironmentCubemap(const VulkanContext* context) {
    try {
        // HDR 파일로부터 512x512 큐브맵 생성
        auto envCubemap = std::make_unique<CubemapTexture>(
            context, 
            "../assets/hdri/environment.hdr", 
            512
        );
        
        // 큐브맵 사용
        VkImageView cubemapView = envCubemap->getImageView();
        VkSampler cubemapSampler = envCubemap->getSampler();
        
        // Descriptor Set에 바인딩 등...
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to create environment cubemap: " << e.what() << std::endl;
    }
}

// 2. 6개 개별 이미지 파일로부터 큐브맵 생성
void createSkyboxCubemap(const VulkanContext* context) {
    try {
        std::array<std::string, 6> skyboxFaces = {
            "../assets/skybox/right.jpg",   // +X
            "../assets/skybox/left.jpg",    // -X
            "../assets/skybox/top.jpg",     // +Y
            "../assets/skybox/bottom.jpg",  // -Y
            "../assets/skybox/front.jpg",   // +Z
            "../assets/skybox/back.jpg"     // -Z
        };
        
        auto skyboxCubemap = std::make_unique<CubemapTexture>(context, skyboxFaces);
        
        // 큐브맵 사용
        VkImageView cubemapView = skyboxCubemap->getImageView();
        VkSampler cubemapSampler = skyboxCubemap->getSampler();
        
        // Descriptor Set에 바인딩 등...
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to create skybox cubemap: " << e.what() << std::endl;
    }
}

// 3. 셰이더에서 큐브맵 사용 예시 (Vertex Shader)
/*
#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragTexCoord;

void main() {
    fragTexCoord = inPosition;
    
    // 스카이박스의 경우 translation을 제거하고 rotation만 적용
    mat4 rotView = mat4(mat3(ubo.view));
    vec4 clipPos = ubo.proj * rotView * vec4(inPosition, 1.0);
    
    // 스카이박스를 항상 가장 멀리 렌더링하기 위해 z = w로 설정
    gl_Position = clipPos.xyww;
}
*/

// 4. 셰이더에서 큐브맵 사용 예시 (Fragment Shader)
/*
#version 450

layout(binding = 1) uniform samplerCube skybox;

layout(location = 0) in vec3 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(skybox, fragTexCoord);
}
*/