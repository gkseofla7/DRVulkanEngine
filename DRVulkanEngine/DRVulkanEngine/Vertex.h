#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include <glm/glm.hpp>


const int MAX_BONES_PER_VERTEX = 4; // 한 정점에 영향을 주는 최대 뼈 개수

struct Vertex {
    glm::vec3 pos;      // 위치 (Position)
    glm::vec3 normal;   // 법선 (Normal) - 조명 계산용
    glm::vec2 texCoord; // 텍스처 좌표 (UV)
    glm::vec3 tangent;
    glm::vec3 bitangent;

    // --- 스켈레탈 애니메이션 데이터 ---
   // int   boneIDs[MAX_BONES_PER_VERTEX];  // 영향을 주는 뼈의 ID
    //float weights[MAX_BONES_PER_VERTEX]; // 각 뼈의 영향력 (가중치)

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions();
};