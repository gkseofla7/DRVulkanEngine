#pragma once
#include <vulkan/vulkan.h>
#include <array>
struct Vertex {
    float pos[3];
    float color[3];
    float texCoord[2];

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};

