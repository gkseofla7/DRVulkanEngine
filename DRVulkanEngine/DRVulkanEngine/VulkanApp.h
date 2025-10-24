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
#include "VulkanPipeline.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include "Vertex.h"
#include "ShaderManager.h"
#include "DescriptorSet.h"
#include "DescriptorPool.h"
#include "UniformBuffer.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
class Camera;
class Texture;
class Model;
// �ﰢ�� ���� ������ ����ü




class VulkanApp {
public:
    VulkanApp();
    ~VulkanApp();
    void run();

private:
    void initWindow();
    void initVulkan();

    void createCommandBuffers();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void createSyncObjects();
    void mainLoop();
    void cleanup();
    void drawFrame();

    void updateUniformBuffer(uint32_t currentImage);

    void loadAssets();

private:
    GLFWwindow* window;
    VulkanContext context_;
    VulkanSwapChain swapChain;
    VulkanPipeline pipeline;

    ShaderManager shaderManager_;

    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    size_t currentFrame = 0;
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    std::unique_ptr<Camera> camera;


    VkDescriptorSetLayout descriptorSetLayout;
    DescriptorPool descriptorPool_;
    std::vector<DescriptorSet> perFrameDescriptorSets_;
    std::vector<DescriptorSet> commonDescriptorSet_;

    std::vector<Model> models_;

	std::map<std::string, UniformBuffer*> uniformBuffers_;
	std::map<std::string, Texture*> textures_;


};