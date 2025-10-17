#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <string>
#include <optional>
#include "VulkanContext.h"
#include "VulkanSwapChain.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// 삼각형 정점 데이터 구조체
struct Vertex {
    float pos[2];
    float color[3];

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};

class VulkanApp {
private:
    GLFWwindow* window;
    VulkanContext context;
    VulkanSwapChain swapChain;
    
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    size_t currentFrame = 0;
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

public:
    void run();

private:
    void initWindow();
    void initVulkan();
    void createGraphicsPipeline();
    void createVertexBuffer();
    void createCommandBuffers();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void createSyncObjects();
    void mainLoop();
    void cleanup();
    void drawFrame();
    static std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);
};