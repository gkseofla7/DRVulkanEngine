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
#include "TextureArray.h"
#include "DescriptorSet.h"
#include "DescriptorPool.h"
#include "UniformBuffer.h"
#include "UniformBufferArray.h"
#include "RenderTarget.h"
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
class Camera;
class Texture;
class Model;

struct UniformBufferScene {
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 view;
    alignas(16) glm::vec3 lightPos;
    alignas(16) glm::vec3 viewPos;
    alignas(4) float exposure;        // HDR ���Ⱚ
    alignas(4) float gamma;           // ���� ������
    alignas(4) float maxWhite;        // �ִ� ����� (Reinhard Extended��)
    alignas(4) int tonemapOperator;   // ����� ������ ���� (0=ACES, 1=Reinhard, 2=ReinhardExt, 3=Exposure)
};

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
    void update();
    void drawFrame();

    void updateUniformBuffer(uint32_t currentImage);
    void handleHDRInput(); // HDR ���� Ű���� �Է� ó��

    void loadAssets();

    static void staticMouseCallback(GLFWwindow* window, double xposIn, double yposIn);
    void mousecallback(GLFWwindow* window, double xposIn, double yposIn);

private:
    GLFWwindow* window;
    VulkanContext context_;
    VulkanSwapChain swapChain_;
    VulkanPipeline defaultPipeline_;
	VulkanPipeline skyboxPipeline_;
	VulkanPipeline tonemappingPipeline_;


    RenderTarget sceneRenderTarget_;

    ShaderManager shaderManager_;

    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    size_t currentFrame = 0;
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    std::unique_ptr<Camera> camera_;

    VkDescriptorSetLayout descriptorSetLayout;
    DescriptorPool descriptorPool_;
    std::vector<DescriptorSet> perFrameDescriptorSets_;
    std::vector<DescriptorSet> commonDescriptorSet_;

    std::vector<Model> models_;
	std::unique_ptr<Model> skyboxModel_;

    std::map<std::string, Resource*> resources_;
    std::unique_ptr<class CubemapTexture> envCubemapTexture_;
    std::unique_ptr<class UniformBuffer> sceneUB_;
    TextureArray textureArray_;
    UniformBufferArray materialUbArray_;
    UniformBufferArray modelUbArray_;
    UniformBufferArray boneUbArray_;
    std::unique_ptr<Texture> defaultTexture_;

    // ī�޶� ����
    float lastFrame;
    float lastX = 800.0f / 2.0;
    float lastY = 600.0f / 2.0;
    bool firstMouse = true;

    // HDR ����� ���� ������
    float hdrExposure = 1.0f;     // �⺻ ���Ⱚ
    float hdrGamma = 2.2f;        // �⺻ ������
    float hdrMaxWhite = 4.0f;     // �ִ� �����
    int tonemapMode = 0;          // 0=ACES, 1=Reinhard, 2=ReinhardExt, 3=Exposure
};