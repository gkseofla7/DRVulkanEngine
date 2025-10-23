#include "Vertex.h"

VkVertexInputBindingDescription Vertex::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 5> Vertex::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};

    // location 0: Position
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    // location 1: Normal
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, normal);

    // location 2: Texture Coordinate
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    // location 3: Tangent
    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, tangent);

    // location 3: Bitangent
    attributeDescriptions[4].binding = 0;
    attributeDescriptions[4].location = 4;
    attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[4].offset = offsetof(Vertex, bitangent);

    //// location 4: Bone IDs
    //attributeDescriptions[4].binding = 0;
    //attributeDescriptions[4].location = 4;
    //attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SINT; // 4개의 정수
    //attributeDescriptions[4].offset = offsetof(Vertex, boneIDs);

    //// location 5: Bone Weights
    //attributeDescriptions[5].binding = 0;
    //attributeDescriptions[5].location = 5;
    //attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT; // 4개의 실수
    //attributeDescriptions[5].offset = offsetof(Vertex, weights);

    return attributeDescriptions;
}