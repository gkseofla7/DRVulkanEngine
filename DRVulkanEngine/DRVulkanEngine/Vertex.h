#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include <glm/glm.hpp>


#define MAX_BONE_INFLUENCE 4

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec3 tangent;
    glm::vec3 bitangent;

    // --- ���̷�Ż �ִϸ��̼� ������ ---
    int boneIDs[MAX_BONE_INFLUENCE];
    float weights[MAX_BONE_INFLUENCE];

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 7> getAttributeDescriptions();
};