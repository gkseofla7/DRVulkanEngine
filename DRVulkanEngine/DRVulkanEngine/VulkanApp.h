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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
class Camera;
// 삼각형 정점 데이터 구조체
struct Vertex {
    float pos[3];
    float color[3];

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};

struct UniformBufferObject {
    glm::mat4 view;
    glm::mat4 proj;
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
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    size_t currentFrame = 0;
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    std::unique_ptr<Camera> camera;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;
public:
    VulkanApp();
    ~VulkanApp();
    void run();

private:
    void initWindow();
    void initVulkan();
    void createGraphicsPipeline();
    void createVertexBuffer();
    void createIndexBuffer();
    void createCommandBuffers();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void createSyncObjects();
    void mainLoop();
    void cleanup();
    void drawFrame();
    static std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);

    void createUniformBuffers();
    void updateUniformBuffer(uint32_t currentImage);
    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSets();
};