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
#include "UniformBuffer.h"
#include "CubemapTexture.h"
#include "GlobalData.h"

VulkanApp::VulkanApp()
    :camera_(std::make_unique<Camera>())
{

}

VulkanApp::~VulkanApp()
{
    cleanup();
}

// VulkanApp 클래스 구현
void VulkanApp::run() {
    initWindow();
    initVulkan();
    glfwSetCursorPosCallback(window, staticMouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    mainLoop();

}

void VulkanApp::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Triangle - Dynamic Rendering", nullptr, nullptr);

    glfwSetWindowUserPointer(window, this);
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
	pipeline_.initialize(&context_, &swapChain, &descriptorPool_, &shaderManager_, pipelineConfig);


    pipelineConfig.pipelineName = "skybox";
    pipelineConfig.vertexShaderPath = "shaders/skybox.vert.spv";
    pipelineConfig.fragmentShaderPath = "shaders/skybox.frag.spv";
    pipelineConfig.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    pipelineConfig.cullMode = VK_CULL_MODE_FRONT_BIT;
    skyboxPipeline_.initialize(&context_, &swapChain, &descriptorPool_, &shaderManager_, pipelineConfig);


    std::unordered_map<std::string, std::vector<std::string>> pipelineDescriptorSetsMap;
    std::string pipelineName = "default";
    std::vector<std::string> descriptorSetNames = {
        "modelUB",
        "modelMaterial",
        "modelTexture"
    };
    pipelineDescriptorSetsMap[pipelineName] = descriptorSetNames;

	resources_["materialTextures"] = &textureArray_;
	resources_["materialData"] = &materialUbArray_;
	resources_["ubo"] = &modelUbArray_;
	resources_["boneData"] = &boneUbArray_;
	resources_["skyboxSampler"] = envCubemapTexture_.get();

	// Default Pipeline Descriptor Set 생성
    {
        std::vector<DescriptorSet> descriptorSets;

        const std::map<uint32_t, std::map<uint32_t, LayoutBindingInfo>>& bindingMap = pipeline_.GetDescriptorSetLayoutBindingMap();
        for (const auto& [setIndex, bindings] : bindingMap) {
			std::vector< VkDescriptorSetLayoutBinding> layoutBindings;
			std::vector<Resource*> requiredResources;
            for (const auto& [bindingIndex, layoutBinding] : bindings) {
                layoutBindings.push_back(layoutBinding.bindingInfo);
                if (resources_.find(layoutBinding.resourceName) != resources_.end())
                {
                    requiredResources.push_back(resources_[layoutBinding.resourceName]);
                }
            }

            DescriptorSet descriptorSet{};
            descriptorSet.initialize(&context_, &descriptorPool_, descriptorPool_.layoutCache_.getLayout(layoutBindings), requiredResources);
            commonDescriptorSet_.push_back(std::move(descriptorSet));
			descriptorSets.push_back(commonDescriptorSet_.back());
		}
        pipeline_.setDescriptorSets(descriptorSets);
    }

    // Skybox Pipeline Descriptor Set 생성
    {
        std::vector<DescriptorSet> descriptorSets;

        const std::map<uint32_t, std::map<uint32_t, LayoutBindingInfo>>& bindingMap = skyboxPipeline_.GetDescriptorSetLayoutBindingMap();
        for (const auto& [setIndex, bindings] : bindingMap) {
            std::vector< VkDescriptorSetLayoutBinding> layoutBindings;
            std::vector<Resource*> requiredResources;
            for (const auto& [bindingIndex, layoutBinding] : bindings) {
                layoutBindings.push_back(layoutBinding.bindingInfo);
                if (resources_.find(layoutBinding.resourceName) != resources_.end())
                {
                    requiredResources.push_back(resources_[layoutBinding.resourceName]);
                }
            }

            DescriptorSet descriptorSet{};
            descriptorSet.initialize(&context_, &descriptorPool_, descriptorPool_.layoutCache_.getLayout(layoutBindings), requiredResources);
            commonDescriptorSet_.push_back(std::move(descriptorSet));
            descriptorSets.push_back(commonDescriptorSet_.back());
        }
        skyboxPipeline_.setDescriptorSets(descriptorSets);
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

	pipeline_.bindPipeline(commandBuffer);

    for (Model& model : models_)
    {
        PushConstantData pushData{};
		model.getPushConstantData(pushData);

        vkCmdPushConstants(
            commandBuffer,
            pipeline_.getPipelineLayout(),
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData),
            &pushData
        );
        model.draw(commandBuffer);
    }
	

	skyboxPipeline_.bindPipeline(commandBuffer);
	skyboxModel_->draw(commandBuffer);


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
    renderFinishedSemaphores.resize(swapChain.getImageCount()) ;
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(context_.getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(context_.getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }

    for (size_t i = 0; i < renderFinishedSemaphores.size(); i++)
    {
        if (vkCreateSemaphore(context_.getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void VulkanApp::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera_->processKeyboard(Camera_Movement::FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera_->processKeyboard(Camera_Movement::BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera_->processKeyboard(Camera_Movement::LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera_->processKeyboard(Camera_Movement::RIGHT, deltaTime);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            //firstMouse = true;
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        drawFrame();
    }
    vkDeviceWaitIdle(context_.getDevice());
}

void VulkanApp::cleanup() {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(context_.getDevice(), imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(context_.getDevice(), inFlightFences[i], nullptr);
    }
    for (size_t i = 0; i < renderFinishedSemaphores.size(); i++)
    {
        vkDestroySemaphore(context_.getDevice(), renderFinishedSemaphores[i], nullptr);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}

void VulkanApp::update()
{
	float dt = 0.016f; // 고정된 델타 타임 (약 60 FPS)

    // TODO. 시간차 생성을 위한 임시 코드, 제거 예정
    static int count = 2;
	static float totalTime = 0.0f;
	float checkTime[2] = { 3.0f, 6.0f };
	totalTime += dt;
    if (count > 0 && totalTime >= checkTime[2-count])
    {
        count--;
        ModelConfig modelConfig{};
        modelConfig.type = ModelType::FromFile;
        modelConfig.modelDirectory = "../assets/models/mouseModel";
        modelConfig.modelFilename = "mouseModel.fbx";
        modelConfig.animationFilenames.push_back("mouseModelAnim.fbx");
        models_.push_back(Model(&context_, modelConfig));
        models_.back().prepareBindless(modelUbArray_, materialUbArray_, boneUbArray_, textureArray_);
    }
    for(auto& descriptorSet : commonDescriptorSet_)
    {
        descriptorSet.updateIfDirty();
	}
    for(Model& model : models_)
    {
        model.update(dt);
	}
}

void VulkanApp::drawFrame() {
    vkWaitForFences(context_.getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);


    update();
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

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[imageIndex]};
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
    glm::mat4 viewMatrix = camera_->getViewMatrix();
    glm::mat4 projMatrix = camera_->getProjectionMatrix(swapChain.getSwapChainExtent().width / (float)swapChain.getSwapChainExtent().height,
        0.1f, 100.0f);

    const float spacing = 2.0f;
    const glm::vec3 scaleFactors(0.02f);
    UniformBufferScene uboScene;
    uboScene.proj = projMatrix;
    uboScene.view = viewMatrix;
    uboScene.lightPos = glm::vec3(0.0, 10.0, 0.0);
    uboScene.viewPos = camera_->getPosition();
    sceneUB_->update(&uboScene);

    for (int i = 0; i < models_.size(); ++i) {

        float xPosition = (static_cast<float>(i) - (static_cast<float>(models_.size() - 1) / 2.0f)) * spacing;

        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(xPosition, 0.0f, 0.0f));
        glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), scaleFactors);

        glm::mat4 worldMatrix = translationMatrix * scalingMatrix;

        models_[i].updateUniformBuffer(worldMatrix, viewMatrix, projMatrix);
    }
}

void VulkanApp::loadAssets() {
	ModelConfig modelConfig{};
	modelConfig.type = ModelType::FromFile;
	modelConfig.modelDirectory = "../assets/models/mouseModel";
	modelConfig.modelFilename = "mouseModel.fbx";
    modelConfig.animationFilenames.push_back("mouseModelAnim.fbx");
	models_.push_back(Model(&context_, modelConfig));
	//models_.push_back(Model(&context_, modelConfig));
	//models_.push_back(Model(&context_, modelConfig));

    for(Model& model : models_)
    {
        model.prepareBindless(modelUbArray_, materialUbArray_, boneUbArray_, textureArray_);
	}
	
    sceneUB_ = std::make_unique<class UniformBuffer>(&context_, sizeof(UniformBufferScene));
    resources_["scene"]= sceneUB_.get();

    defaultTexture_ = std::make_unique<Texture>(&context_, "../assets/images/minion.jpg");
    textureArray_.addDefaultTexture(defaultTexture_.get());

    envCubemapTexture_ = std::make_unique<CubemapTexture>(
        &context_,
        "../assets/hdri/german_town_street_4k.hdr",
        512
    );

    ModelConfig skyboxModelConfig{};
    skyboxModelConfig.type = ModelType::Box;
	skyboxModel_= std::make_unique<Model>(&context_, skyboxModelConfig);
}

void VulkanApp::mousecallback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    // 처음 콜백이 호출될 때는 lastX, lastY를 현재 마우스 위치로 설정
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    // 이전 프레임과 현재 프레임의 마우스 위치 차이를 계산
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Y 좌표는 화면 위쪽으로 갈수록 값이 작아지므로 반대로 계산

    // 다음 프레임을 위해 현재 위치를 저장
    lastX = xpos;
    lastY = ypos;

    
    // glfwGetMouseButton 함수로 현재 마우스 버튼 상태를 가져올 수 있습니다.
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        camera_->processMouseMovement(xoffset, yoffset);
    }
}

void VulkanApp::staticMouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
    // 1. 창에 등록된 '집 주소'(this 포인터)를 가져옵니다.
    auto* app = static_cast<VulkanApp*>(glfwGetWindowUserPointer(window));

    if (app) {
        app->mousecallback(window, xposIn, yposIn);
    }
}
