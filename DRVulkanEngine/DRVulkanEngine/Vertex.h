#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include <glm/glm.hpp>


#define MAX_BONE_INFLUENCE 4

struct Vertex {
    glm::vec3 pos;      // 위치 (Position)
    glm::vec3 normal;   // 법선 (Normal) - 조명 계산용
    glm::vec2 texCoord; // 텍스처 좌표 (UV)
    glm::vec3 tangent;
    glm::vec3 bitangent;

    // --- 스켈레탈 애니메이션 데이터 ---
    int boneIDs[MAX_BONE_INFLUENCE];
    float weights[MAX_BONE_INFLUENCE];

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 7> getAttributeDescriptions();
};