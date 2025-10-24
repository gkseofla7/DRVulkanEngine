#include "VulkanApp.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <set>
#include <algorithm>
#include <unordered_map>
#include "Camera.h"
#include "Texture.h"
#include "Model.h"
#include "ModelLoader.h"
#include "Material.h"
#include "Shader.h"

VulkanApp::VulkanApp()
    :camera(std::make_unique<Camera>(glm::vec3{ 0.0f, 3.0f, 5.0f },
        glm::vec3{ 0.0f, 2.0f, 0.0f },
        glm::vec3{ 0.0f, 1.0f, 0.0f }))
{

}

VulkanApp::~VulkanApp() = default;

// VulkanApp 클래스 구현
void VulkanApp::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void VulkanApp::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Triangle - Dynamic Rendering", nullptr, nullptr);
}

void VulkanApp::initVulkan() {
	context_.initialize(window);

    //createDescriptorSetLayout();
    swapChain.initialize(&context_, window);
	shaderManager_.initialize(&context_);

    loadAssets();
    descriptorPool_.initialize(&context_);
	// Pipeline 초기화
	PipelineConfig pipelineConfig{};
	pipelineConfig.pipelineName = "default";
	pipelineConfig.vertexShaderPath = "shaders/shader.vert.spv";
	pipelineConfig.fragmentShaderPath = "shaders/shader.frag.spv";
	pipeline.initialize(&context_, &swapChain, &descriptorPool_, &shaderManager_, pipelineConfig);

    std::unordered_map<std::string, std::vector<std::string>> pipelineDescriptorSetsMap;
    std::string pipelineName = "default";
    std::vector<std::string> descriptorSetNames = {
        "modelUB",
        "modelMaterial",
        "modelTexture"
    };
    pipelineDescriptorSetsMap[pipelineName] = descriptorSetNames;
    {
        const std::map<uint32_t, std::map<uint32_t, LayoutBindingInfo>>& bindingMap = pipeline.GetDescriptorSetLayoutBindingMap();
        for (const auto& [setIndex, bindings] : bindingMap) {
			std::vector< VkDescriptorSetLayoutBinding> layoutBindings;
			std::vector<Resource*> requiredResources;
            for (const auto& [bindingIndex, layoutBinding] : bindings) {
                layoutBindings.push_back(layoutBinding.bindingInfo);
                if(uniformBuffers_.find(layoutBinding.resourceName) != uniformBuffers_.end())
                {
                    requiredResources.push_back(uniformBuffers_[layoutBinding.resourceName]);
				}
                else if (textures_.find(layoutBinding.resourceName) != textures_.end())
                {
                    requiredResources.push_back(textures_[layoutBinding.resourceName]);
                }
            }

            DescriptorSet descriptorSet{};
            descriptorSet.initialize(&context_, &descriptorPool_, descriptorPool_.layoutCache_.getLayout(layoutBindings), requiredResources);
            commonDescriptorSet_.push_back(descriptorSet);
		}
    }
    createCommandBuffers();
    createSyncObjects();
}


void VulkanApp::createCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = context_.getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    if (vkAllocateCommandBuffers(context_.getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void VulkanApp::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkImageMemoryBarrier imageBarrier{};
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.image = swapChain.getSwapChainImages()[imageIndex];
    imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrier.subresourceRange.baseMipLevel = 0;
    imageBarrier.subresourceRange.levelCount = 1;
    imageBarrier.subresourceRange.baseArrayLayer = 0;
    imageBarrier.subresourceRange.layerCount = 1;
    imageBarrier.srcAccessMask = 0;
    imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &imageBarrier);

    // Dynamic Rendering 시작
    auto renderingInfo = swapChain.getRenderingInfo(imageIndex);
    vkCmdBeginRendering(commandBuffer, &renderingInfo);

	pipeline.bindPipeline(commandBuffer);
    for (Model& model : models_)
    {
        model.draw(commandBuffer);
    }
	
    vkCmdEndRendering(commandBuffer);

    // 이미지 레이아웃 전환 (COLOR_ATTACHMENT_OPTIMAL -> PRESENT_SRC)
    imageBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    imageBarrier.dstAccessMask = 0;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &imageBarrier);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void VulkanApp::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(context_.getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(context_.getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(context_.getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void VulkanApp::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        drawFrame();
    }
    vkDeviceWaitIdle(context_.getDevice());
}

void VulkanApp::cleanup() {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(context_.getDevice(), renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(context_.getDevice(), imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(context_.getDevice(), inFlightFences[i], nullptr);
    }

    // SwapChain과 VulkanContext는 자동으로 정리됨 (RAII)

    glfwDestroyWindow(window);
    glfwTerminate();
}

void VulkanApp::drawFrame() {
    vkWaitForFences(context_.getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(context_.getDevice(), swapChain.getSwapChain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        swapChain.recreate();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(context_.getDevice(), 1, &inFlightFences[currentFrame]);

    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);
    updateUniformBuffer(currentFrame);


    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(context_.getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain.getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(context_.getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        swapChain.recreate();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApp::updateUniformBuffer(uint32_t currentImage) {
    UniformBufferObject ubo{};

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    glm::vec3 scaleFactors = glm::vec3(0.02f);

    ubo.world = glm::scale(modelMatrix, scaleFactors);
    ubo.view = camera->getViewMatrix();
    ubo.proj = camera->getProjectionMatrix(45.0f,
        swapChain.getSwapChainExtent().width / (float)swapChain.getSwapChainExtent().height,
        0.1f, 10.0f);
	models_[0].updateUniformBuffer(ubo.world, ubo.view, ubo.proj);
}

//void VulkanApp::createDescriptorSetLayout() {
//    // 1. Uniform Buffer Object(UBO) 바인딩 정의 (binding = 0)
//    VkDescriptorSetLayoutBinding uboLayoutBinding{};
//    uboLayoutBinding.binding = 0;
//    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//    uboLayoutBinding.descriptorCount = 1;
//    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // 버텍스 셰이더에서 사용
//    uboLayoutBinding.pImmutableSamplers = nullptr;
//
//    // 2. Combined Image Sampler 바인딩 정의 (binding = 1)
//    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
//    samplerLayoutBinding.binding = 1; // 셰이더의 layout(binding = 1)과 일치
//    samplerLayoutBinding.descriptorCount = 1;
//    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//    samplerLayoutBinding.pImmutableSamplers = nullptr;
//    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // 프래그먼트 셰이더에서 사용
//
//    // 3. 위에서 정의한 바인딩들을 배열에 담습니다.
//    std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding, samplerLayoutBinding };
//
//    // 4. 레이아웃 생성 정보 업데이트
//    VkDescriptorSetLayoutCreateInfo layoutInfo{};
//    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size()); // 바인딩 개수를 2로 설정
//    layoutInfo.pBindings = bindings.data(); // 바인딩 배열의 포인터를 전달
//
//    if (vkCreateDescriptorSetLayout(context_.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create descriptor set layout!");
//    }
//}


void VulkanApp::loadAssets() {
	models_.push_back(Model(&context_, "../assets/models/mouseModel", "mouseModel.fbx"));
	models_[0].prepareBindless(uniformBuffers_, textures_);
}