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
#include "VulkanUtils.h"

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
    swapChain_.initialize(&context_, window);
    sceneRenderTarget_.initialize(
        &context_,
        swapChain_.getSwapChainExtent(),
        VK_FORMAT_R16G16B16A16_SFLOAT,
        swapChain_.getDepthFormat()
    );
	shaderManager_.initialize(&context_);

    loadAssets();
    descriptorPool_.initialize(&context_);
	// Pipeline 초기화
	PipelineConfig pipelineConfig{};
	pipelineConfig.pipelineName = "default";
	pipelineConfig.vertexShaderPath = "shaders/shader.vert.spv";
	pipelineConfig.fragmentShaderPath = "shaders/shader.frag.spv";
    pipelineConfig.colorAttachmentFormat = sceneRenderTarget_.getColorFormat(); // 예: R16G16B16A16_SFLOAT
    pipelineConfig.depthAttachmentFormat = sceneRenderTarget_.getDepthFormat();
	defaultPipeline_.initialize(&context_, &swapChain_, &descriptorPool_, &shaderManager_, pipelineConfig);


    pipelineConfig.pipelineName = "skybox";
    pipelineConfig.vertexShaderPath = "shaders/skybox.vert.spv";
    pipelineConfig.fragmentShaderPath = "shaders/skybox.frag.spv";
    pipelineConfig.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    pipelineConfig.cullMode = VK_CULL_MODE_FRONT_BIT;
    pipelineConfig.colorAttachmentFormat = sceneRenderTarget_.getColorFormat(); // 예: R16G16B16A16_SFLOAT
    pipelineConfig.depthAttachmentFormat = sceneRenderTarget_.getDepthFormat();
    skyboxPipeline_.initialize(&context_, &swapChain_, &descriptorPool_, &shaderManager_, pipelineConfig);

    PipelineConfig tonemappingConfig;
    tonemappingConfig.pipelineName = "Tonemapping";
    tonemappingConfig.vertexShaderPath = "shaders/fullscreen.vert.spv";
    tonemappingConfig.fragmentShaderPath = "shaders/tonemapping.frag.spv";
    tonemappingConfig.depthTestEnable = false;
    tonemappingConfig.depthWriteEnable = false;
    tonemappingConfig.cullMode = VK_CULL_MODE_NONE;
    tonemappingConfig.useVertexInput = false;
    tonemappingConfig.colorAttachmentFormat = swapChain_.getSwapChainImageFormat();
    tonemappingConfig.depthAttachmentFormat = VK_FORMAT_UNDEFINED;

    tonemappingPipeline_.initialize(&context_, &swapChain_, &descriptorPool_, &shaderManager_, tonemappingConfig);


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
	resources_["hdrSceneTexture"] = sceneRenderTarget_.getColorTexture();

	// Default Pipeline Descriptor Set 생성
    {
        std::vector<DescriptorSet> descriptorSets;

        const std::map<uint32_t, std::map<uint32_t, LayoutBindingInfo>>& bindingMap = defaultPipeline_.GetDescriptorSetLayoutBindingMap();
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
        defaultPipeline_.setDescriptorSets(descriptorSets);
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

    // Tonemapping Pipeline Descriptor Set 생성
    {
        std::vector<DescriptorSet> descriptorSets;

        const std::map<uint32_t, std::map<uint32_t, LayoutBindingInfo>>& bindingMap = tonemappingPipeline_.GetDescriptorSetLayoutBindingMap();
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
        tonemappingPipeline_.setDescriptorSets(descriptorSets);
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
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR
    imageBarrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.image = swapChain_.getSwapChainImages()[imageIndex];
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
    // ====================================================================
	// 렌더링 패스 1: 장면 렌더링
    // ====================================================================
    {
        // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR
        sceneRenderTarget_.getColorTexture()->transitionLayout_Cmd(
            commandBuffer,
            VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR);

        auto renderingInfo = sceneRenderTarget_.getRenderingInfo();
        //auto renderingInfo = swapChain_.getRenderingInfo(imageIndex);
        vkCmdBeginRendering(commandBuffer, &renderingInfo);

        defaultPipeline_.bindPipeline(commandBuffer);

        for (Model& model : models_)
        {
            PushConstantData pushData{};
            model.getPushConstantData(pushData);

            vkCmdPushConstants(
                commandBuffer,
                defaultPipeline_.getPipelineLayout(),
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
    }
    // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL -> VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR
    sceneRenderTarget_.getColorTexture()->transitionLayout_Cmd(
        commandBuffer,
        VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR);

    // ====================================================================
    // 렌더링 패스 2: 톤 매핑
    // ====================================================================
    {
        // 렌더링 정보를 이제 swapChain_에서 가져옵니다.
        auto renderingInfo = swapChain_.getRenderingInfo(imageIndex);
        renderingInfo.pDepthAttachment = nullptr; // 톤 매핑은 깊이 버퍼가 필요 없습니다.

        vkCmdBeginRendering(commandBuffer, &renderingInfo);

        // 톤 매핑용 파이프라인과 디스크립터 셋 바인딩
        tonemappingPipeline_.bindPipeline(commandBuffer);

        // tonemapping.frag에서 exposure 기대하지만 전달 안됨
        //struct PushConstants { float exposure; } pushConstants;
        vkCmdPushConstants(commandBuffer, tonemappingPipeline_.getPipelineLayout(),
                          VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &hdrExposure);

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRendering(commandBuffer);
    }


    // 이미지 레이아웃 전환 (COLOR_ATTACHMENT_OPTIMAL -> PRESENT_SRC)
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL -> VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR
    imageBarrier.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
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
    renderFinishedSemaphores.resize(swapChain_.getImageCount()) ;
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
    std::cout << "=== HDR Controls ===" << std::endl;
    std::cout << "Q/E: Decrease/Increase Exposure" << std::endl;
    std::cout << "R/T: Decrease/Increase Gamma" << std::endl;
    std::cout << "1: ACES Filmic Tonemap" << std::endl;
    std::cout << "2: Reinhard Tonemap" << std::endl;
    std::cout << "3: Reinhard Extended Tonemap" << std::endl;
    std::cout << "4: Simple Exposure Tonemap" << std::endl;
    std::cout << "===================" << std::endl;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 카메라 입력 처리
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera_->processKeyboard(Camera_Movement::FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera_->processKeyboard(Camera_Movement::BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera_->processKeyboard(Camera_Movement::LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera_->processKeyboard(Camera_Movement::RIGHT, deltaTime);

        // HDR 입력 처리
        handleHDRInput();

        // 마우스 커서 제어
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
	float checkTime[2] = { 0.0f, 0.0f };
	totalTime += dt;
    if (count > 0 && totalTime >= checkTime[2-count])
    {
        count--;
        ModelConfig modelConfig{};
        modelConfig.type = ModelType::FromFile;
        modelConfig.modelDirectory = "../assets/models/mouseModel";
        modelConfig.modelFilename = "mouseModel.fbx";
        modelConfig.animationFilenames.push_back("Hip Hop Dancing_cleaned.fbx");
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
    VkResult result = vkAcquireNextImageKHR(context_.getDevice(), swapChain_.getSwapChain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        swapChain_.recreate();
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

    VkSwapchainKHR swapChains[] = {swapChain_.getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(context_.getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        swapChain_.recreate();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApp::updateUniformBuffer(uint32_t currentImage) {
    glm::mat4 viewMatrix = camera_->getViewMatrix();
    glm::mat4 projMatrix = camera_->getProjectionMatrix(swapChain_.getSwapChainExtent().width / (float)swapChain_.getSwapChainExtent().height,
        0.1f, 100.0f);

    const float spacing = 2.0f;
    const glm::vec3 scaleFactors(0.02f);
    
    UniformBufferScene uboScene;
    uboScene.proj = projMatrix;
    uboScene.view = viewMatrix;
    uboScene.lightPos = glm::vec3(0.0, 10.0, 0.0);
    uboScene.viewPos = camera_->getPosition();
    
    // HDR 관련 파라미터 업데이트
    uboScene.exposure = hdrExposure;
    uboScene.gamma = hdrGamma;
    uboScene.maxWhite = hdrMaxWhite;
    uboScene.tonemapOperator = tonemapMode;
    
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
    modelConfig.animationFilenames.push_back("Hip Hop Dancing_cleaned.fbx");
	models_.push_back(Model(&context_, modelConfig));

    for(Model& model : models_)
    {
        model.prepareBindless(modelUbArray_, materialUbArray_, boneUbArray_, textureArray_);
	}
	
    sceneUB_ = std::make_unique<class UniformBuffer>(&context_, sizeof(UniformBufferScene));
    resources_["scene"]= sceneUB_.get();

    defaultTexture_ = std::make_unique<Texture>(&context_, "../assets/images/minion.jpg");
    textureArray_.addDefaultTexture(defaultTexture_.get());

    // HDR 환경맵 로드 - 더 큰 해상도로 개선
    envCubemapTexture_ = std::make_unique<CubemapTexture>(
        &context_,
        "../assets/hdri/bryanston_park_sunrise_4k.hdr",
        1024  // 더 높은 해상도로 변경 (512 -> 1024)
    );

    // HDR 지원 여부 확인 및 로그
    std::cout << "HDR Environment map loaded: " << "../assets/hdri/german_town_street_4k.hdr" << std::endl;

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

void VulkanApp::handleHDRInput() {
    // HDR 노출값 조정 (Q/E 키)
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        hdrExposure -= 0.02f;
        if (hdrExposure < 0.1f) hdrExposure = 0.1f;
        std::cout << "HDR Exposure: " << hdrExposure << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        hdrExposure += 0.02f;
        if (hdrExposure > 5.0f) hdrExposure = 5.0f;
        std::cout << "HDR Exposure: " << hdrExposure << std::endl;
    }

    // 감마 조정 (R/T 키)
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        hdrGamma -= 0.01f;
        if (hdrGamma < 1.0f) hdrGamma = 1.0f;
        std::cout << "HDR Gamma: " << hdrGamma << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        hdrGamma += 0.01f;
        if (hdrGamma > 3.0f) hdrGamma = 3.0f;
        std::cout << "HDR Gamma: " << hdrGamma << std::endl;
    }

    // 톤맵핑 모드 변경 (1,2,3,4 키)
    static bool key1Pressed = false, key2Pressed = false, key3Pressed = false, key4Pressed = false;
    
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !key1Pressed) {
        tonemapMode = 0; // ACES
        key1Pressed = true;
        std::cout << "Tonemap Mode: ACES Filmic" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE) key1Pressed = false;

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !key2Pressed) {
        tonemapMode = 1; // Reinhard
        key2Pressed = true;
        std::cout << "Tonemap Mode: Reinhard" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE) key2Pressed = false;

    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !key3Pressed) {
        tonemapMode = 2; // Reinhard Extended
        key3Pressed = true;
        std::cout << "Tonemap Mode: Reinhard Extended" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE) key3Pressed = false;

    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && !key4Pressed) {
        tonemapMode = 3; // Simple Exposure
        key4Pressed = true;
        std::cout << "Tonemap Mode: Simple Exposure" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE) key4Pressed = false;
}
