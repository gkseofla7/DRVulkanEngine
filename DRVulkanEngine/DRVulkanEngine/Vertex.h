#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include <glm/glm.hpp>


const int MAX_BONES_PER_VERTEX = 4; // �� ������ ������ �ִ� �ִ� �� ����

struct Vertex {
    glm::vec3 pos;      // ��ġ (Position)
    glm::vec3 normal;   // ���� (Normal) - ���� ����
    glm::vec2 texCoord; // �ؽ�ó ��ǥ (UV)
    glm::vec3 tangent;
    glm::vec3 bitangent;

    // --- ���̷�Ż �ִϸ��̼� ������ ---
   // int   boneIDs[MAX_BONES_PER_VERTEX];  // ������ �ִ� ���� ID
    //float weights[MAX_BONES_PER_VERTEX]; // �� ���� ����� (����ġ)

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions();
};