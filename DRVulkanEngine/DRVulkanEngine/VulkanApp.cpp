#include "VulkanApp.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <set>
#include <algorithm>
#include "Camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
// 삼각형 정점 데이터
// 정육면체를 구성하는 8개의 고유한 정점(Vertex) 데이터
// 각 면이 고유한 색상을 갖도록 정의된 24개의 정점 데이터
const std::vector<Vertex> vertices = {
    // 앞면 (빨강) - 정점 인덱스 0-3
    // pos                      color                  texCoord
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // 0: Bottom-left
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // 1: Bottom-right
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // 2: Top-right
    {{-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 3: Top-left

    // 뒷면 (파랑) - 정점 인덱스 4-7
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 4: Bottom-right
    {{ 0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 5: Bottom-left
    {{ 0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // 6: Top-left
    {{-0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // 7: Top-right

    // 왼쪽 면 (초록) - 정점 인덱스 8-11
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // 8: Bottom-right
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // 9: Bottom-left
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // 10: Top-left
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // 11: Top-right

    // 오른쪽 면 (노랑) - 정점 인덱스 12-15
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // 12: Bottom-left
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // 13: Top-left
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // 14: Top-right
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // 15: Bottom-right

    // 윗면 (시안) - 정점 인덱스 16-19
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // 16: Bottom-left
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // 17: Bottom-right
    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // 18: Top-right
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 19: Top-left

    // 아랫면 (마젠타) - 정점 인덱스 20-23
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // 20: Top-left
    {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 21: Bottom-left
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 22: Bottom-right
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}  // 23: Top-right
};

// 위 'vertices' 배열의 인덱스를 사용하여 6개의 면(삼각형 12개)을 그립니다.
// 주석: 와인딩 순서(Winding Order)는 반시계 방향(Counter-Clockwise)을 기준으로 합니다.
// 이는 뒷면 컬링(Back-face Culling)을 올바르게 처리하기 위함입니다.
const std::vector<uint16_t> indices = {
    // 앞면 (CCW)
    0, 1, 2,   0, 2, 3,
    // 뒷면 (CCW)
    4, 5, 6,   6, 7, 4,
    // 왼쪽 면 (CCW)
    8, 9, 10,  10, 11, 8,

    // 오른쪽 면 (CCW)
    12, 15, 14,  14, 13, 12,

    // 윗면 (CCW)
    16, 17, 18,  18, 19, 16,

    // 아랫면 (CCW)
    20, 23, 22,  22, 21, 20
};
VulkanApp::VulkanApp()
    :camera(std::make_unique<Camera>(glm::vec3{ 3.0f, 3.0f, -3.0f },
        glm::vec3{ 0.0f, 0.0f, 0.0f },
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
    context.createInstance();
    context.createSurface(window);
    context.pickPhysicalDevice();
    context.createLogicalDevice();
    context.createCommandPool();
    createDescriptorSetLayout();
    swapChain.initialize(&context, window); // RenderPass 없이 SwapChain 초기화
	pipeline.initialize(&context, &swapChain, &descriptorSetLayout);
    createUniformBuffers();
    createTextureImage();
    createTextureSampler();
    createDescriptorPool();
    createDescriptorSets();

    createVertexBuffer();
    createIndexBuffer();
    createCommandBuffers();
    createSyncObjects();
}

void VulkanApp::createVertexBuffer() {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(vertices[0]) * vertices.size();
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(context.getDevice(), &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context.getDevice(), vertexBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = context.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(context.getDevice(), &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(context.getDevice(), vertexBuffer, vertexBufferMemory, 0);

    void* data;
    vkMapMemory(context.getDevice(), vertexBufferMemory, 0, bufferInfo.size, 0, &data);
    memcpy(data, vertices.data(), (size_t) bufferInfo.size);
    vkUnmapMemory(context.getDevice(), vertexBufferMemory);
}

void VulkanApp::createIndexBuffer() {
    // 1. 버퍼의 크기를 'indices' 벡터를 기준으로 계산합니다.
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    // 2. 버퍼의 용도를 VK_BUFFER_USAGE_INDEX_BUFFER_BIT로 설정합니다.
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // 'indexBuffer' 핸들에 버퍼를 생성합니다.
    if (vkCreateBuffer(context.getDevice(), &bufferInfo, nullptr, &indexBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create index buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context.getDevice(), indexBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = context.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // 'indexBufferMemory' 핸들에 메모리를 할당합니다.
    if (vkAllocateMemory(context.getDevice(), &allocInfo, nullptr, &indexBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate index buffer memory!");
    }

    // 생성된 버퍼와 메모리를 연결(바인딩)합니다.
    vkBindBufferMemory(context.getDevice(), indexBuffer, indexBufferMemory, 0);

    // 이제 데이터를 버퍼에 복사합니다.
    void* data;
    vkMapMemory(context.getDevice(), indexBufferMemory, 0, bufferSize, 0, &data);
    // 3. 원본 데이터를 'vertices' 대신 'indices' 벡터에서 가져옵니다.
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(context.getDevice(), indexBufferMemory);
}


void VulkanApp::createCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = context.getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    if (vkAllocateCommandBuffers(context.getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void VulkanApp::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    // 이미지 레이아웃 전환 (UNDEFINED -> COLOR_ATTACHMENT_OPTIMAL)
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

    vkCmdBindDescriptorSets(commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline.getPipelineLayout(),
        0, // firstSet: 디스크립터 셋 레이아웃 배열의 시작 인덱스
        1, // descriptorSetCount: 바인딩할 디스크립터 셋의 개수
        &descriptorSets[currentFrame], // pDescriptorSets: 바인딩할 디스크립터 셋의 포인터
        0, // dynamicOffsetCount
        nullptr); // pDynamicOffsets
    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);


    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

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
        if (vkCreateSemaphore(context.getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(context.getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(context.getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void VulkanApp::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        drawFrame();
    }
    vkDeviceWaitIdle(context.getDevice());
}

void VulkanApp::cleanup() {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(context.getDevice(), renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(context.getDevice(), imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(context.getDevice(), inFlightFences[i], nullptr);
    }

    vkDestroyBuffer(context.getDevice(), vertexBuffer, nullptr);
    vkFreeMemory(context.getDevice(), vertexBufferMemory, nullptr);

    // SwapChain과 VulkanContext는 자동으로 정리됨 (RAII)

    glfwDestroyWindow(window);
    glfwTerminate();
}

void VulkanApp::drawFrame() {
    vkWaitForFences(context.getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(context.getDevice(), swapChain.getSwapChain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        swapChain.recreate();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(context.getDevice(), 1, &inFlightFences[currentFrame]);

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

    if (vkQueueSubmit(context.getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
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

    result = vkQueuePresentKHR(context.getPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        swapChain.recreate();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

std::vector<char> VulkanApp::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule VulkanApp::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(context.getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

// VulkanApp.cpp

void VulkanApp::createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    size_t swapChainImageCount = swapChain.getImageCount();

    // 스왑체인 이미지 개수만큼 버퍼와 메모리 핸들을 담을 공간을 미리 확보합니다.
    uniformBuffers.resize(swapChainImageCount);
    uniformBuffersMemory.resize(swapChainImageCount);
    uniformBuffersMapped.resize(swapChainImageCount);

    // 각 스왑체인 이미지에 대해 루프를 돕니다.
    for (size_t i = 0; i < swapChainImageCount; i++) {
        // 버퍼 생성
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(context.getDevice(), &bufferInfo, nullptr, &uniformBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create uniform buffer!");
        }

        // 메모리 요구사항 확인 및 할당
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(context.getDevice(), uniformBuffers[i], &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        //CPU에서 접근 가능하고(HOST_VISIBLE), 쓰기 즉시 GPU에 반영되는(HOST_COHERENT) 속성으로 메모리를 찾습니다.
        allocInfo.memoryTypeIndex = context.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(context.getDevice(), &allocInfo, nullptr, &uniformBuffersMemory[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate uniform buffer memory!");
        }

        // 버퍼와 메모리 바인딩
        vkBindBufferMemory(context.getDevice(), uniformBuffers[i], uniformBuffersMemory[i], 0);

        //핵심: 메모리를 영구적으로 매핑하고, 그 주소를 uniformBuffersMapped에 저장합니다.
        // unmap을 호출하지 않으므로, 이 주소는 프로그램 종료 전까지 계속 유효합니다.
        vkMapMemory(context.getDevice(), uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void VulkanApp::updateUniformBuffer(uint32_t currentImage) {
    UniformBufferObject ubo{};

    // 1. 카메라로부터 View 매트릭스 가져오기
    ubo.view = camera->getViewMatrix();

    // 2. 카메라로부터 Projection 매트릭스 가져오기
    // 시야각: 45도, 화면비: 스왑체인 비율, Near/Far: 0.1/10.0
    ubo.proj = camera->getProjectionMatrix(45.0f,
        swapChain.getSwapChainExtent().width / (float)swapChain.getSwapChainExtent().height,
        0.1f, 10.0f);

    // 3. UBO 데이터를 GPU 메모리에 복사
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void VulkanApp::createDescriptorSetLayout() {
    // 1. Uniform Buffer Object(UBO) 바인딩 정의 (binding = 0)
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // 버텍스 셰이더에서 사용
    uboLayoutBinding.pImmutableSamplers = nullptr;

    // 2. Combined Image Sampler 바인딩 정의 (binding = 1)
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1; // 셰이더의 layout(binding = 1)과 일치
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // 프래그먼트 셰이더에서 사용

    // 3. 위에서 정의한 바인딩들을 배열에 담습니다.
    std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding, samplerLayoutBinding };

    // 4. 레이아웃 생성 정보 업데이트
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size()); // 바인딩 개수를 2로 설정
    layoutInfo.pBindings = bindings.data(); // 바인딩 배열의 포인터를 전달

    if (vkCreateDescriptorSetLayout(context.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}
void VulkanApp::createDescriptorPool() {
    std::vector<VkDescriptorPoolSize> poolSizes(2);

    // UBO를 위한 풀 크기
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChain.getImageCount());

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChain.getImageCount());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(swapChain.getImageCount());

    if (vkCreateDescriptorPool(context.getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}
void VulkanApp::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(swapChain.getImageCount(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChain.getImageCount());
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(swapChain.getImageCount());
    if (vkAllocateDescriptorSets(context.getDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    // 할당된 각 디스크립터 셋에 실제 리소스 정보를 기록(업데이트)합니다.
    for (size_t i = 0; i < swapChain.getImageCount(); i++) {
        // 1. UBO 정보 설정 (binding = 0)
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        // 2. 텍스처 이미지 및 샘플러 정보 설정 (binding = 1) 
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // 셰이더가 읽기 위한 레이아웃
        imageInfo.imageView = textureView_; // 생성해둔 텍스처 이미지 뷰
        imageInfo.sampler = textureSampler_;     // 생성해둔 텍스처 샘플러

        // 3. 두 개의 쓰기 작업을 배열에 담습니다.
        std::vector<VkWriteDescriptorSet> descriptorWrites(2);

        // UBO 쓰기 작업 설정
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        // 텍스처 샘플러 쓰기 작업 설정 (추가된 부분)
        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1; // 셰이더의 layout(binding = 1)과 일치
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo; // 버퍼가 아닌 이미지 정보를 연결

        // 4. 두 개의 쓰기 작업을 한번에 업데이트합니다.
        vkUpdateDescriptorSets(context.getDevice(),
            static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(),
            0, nullptr);
    }
}

void VulkanApp::createTextureImage() {
    int texWidth, texHeight, texChannels;
    // stbi_load 함수로 이미지 파일을 불러옵니다.
    // STBI_rgb_alpha 플래그는 이미지를 강제로 RGBA(4채널) 형식으로 로드합니다.
    stbi_uc* pixels = stbi_load("../assets/images/minion.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    // CPU에서 접근 가능하고(HOST_VISIBLE), 복사의 원본(TRANSFER_SRC)이 될 버퍼를 생성합니다.
    createBuffer(imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    // Staging Buffer의 메모리를 매핑하여 CPU의 픽셀 데이터를 복사합니다.
    void* data;
    vkMapMemory(context.getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(context.getDevice(), stagingBufferMemory);

    // 원본 픽셀 데이터는 이제 CPU 메모리에서 해제해도 됩니다.
    stbi_image_free(pixels);


// 이미지 생성
    createImage(texWidth, texHeight,
        VK_FORMAT_R8G8B8A8_SRGB, // SRGB 포맷은 색상을 더 자연스럽게 표현합니다.
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, // 복사 대상 + 셰이더 샘플링용
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // 최고의 성능을 위해 GPU 전용 메모리에 생성
        texture_,
        textureMemory_);

    // 레이아웃 변경 및 데이터 복사
    // (1) UNDEFINED -> TRANSFER_DST_OPTIMAL (복사 받기 좋은 레이아웃으로 변경)
    transitionImageLayout(texture_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // (2) Staging Buffer에서 최종 이미지로 픽셀 데이터 복사
    copyBufferToImage(stagingBuffer, texture_, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    // (3) TRANSFER_DST_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL (셰이더가 읽기 좋은 레이아웃으로 변경)
    transitionImageLayout(texture_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // 복사가 끝났으므로 Staging Buffer는 파괴합니다.
    vkDestroyBuffer(context.getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(context.getDevice(), stagingBufferMemory, nullptr);

    // 이미지 뷰 생성
    // 셰이더는 VkImage가 아닌 VkImageView를 통해 이미지에 접근합니다.
    textureView_ = createImageView(texture_, VK_FORMAT_R8G8B8A8_SRGB);
}

// 이 함수들은 보통 Vulkan Context나 Base 클래스의 멤버 함수로 만듭니다.
// device와 physicalDevice가 멤버 변수로 존재한다고 가정합니다.

void VulkanApp::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    // 1. 버퍼 생성 정보(CreateInfo) 정의
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size; // 버퍼의 크기 (바이트 단위)
    bufferInfo.usage = usage; // 버퍼의 용도 (예: 정점 버퍼, 복사 원본 등)
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // 단일 큐에서만 사용 (간단한 설정)

    // 2. 버퍼 핸들 생성
    if (vkCreateBuffer(context.getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    // 3. 버퍼에 필요한 메모리 요구사항 쿼리
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context.getDevice(), buffer, &memRequirements);

    // 4. 메모리 할당 정보(AllocateInfo) 정의
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size; // 필요한 메모리 크기
    // 요구사항과 속성을 만족하는 적절한 메모리 타입을 찾습니다.
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    // 5. GPU 메모리 할당
    if (vkAllocateMemory(context.getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    // 6. 할당된 메모리를 버퍼에 바인딩(연결)
    vkBindBufferMemory(context.getDevice(), buffer, bufferMemory, 0);
}

void VulkanApp::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    // 1. 이미지 생성 정보(CreateInfo) 정의
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1; // 2D 텍스처이므로 depth는 1
    imageInfo.mipLevels = 1; // 지금은 밉맵을 사용하지 않음
    imageInfo.arrayLayers = 1; // 배열 텍스처가 아님
    imageInfo.format = format; // 이미지의 픽셀 포맷 (예: RGBA)
    imageInfo.tiling = tiling; // 텍셀 배치 방식 (Optimal이 GPU에 최적)
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // 초기 레이아웃은 신경 쓰지 않음
    imageInfo.usage = usage; // 이미지의 용도 (예: 텍스처 샘플링, 복사 대상)
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // 멀티샘플링 사용 안 함

    // 2. 이미지 핸들 생성
    if (vkCreateImage(context.getDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    // 3. 이미지에 필요한 메모리 요구사항 쿼리
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(context.getDevice(), image, &memRequirements);

    // 4. 메모리 할당 정보(AllocateInfo) 정의
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    // 5. GPU 메모리 할당
    if (vkAllocateMemory(context.getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    // 6. 할당된 메모리를 이미지에 바인딩(연결)
    vkBindImageMemory(context.getDevice(), image, imageMemory, 0);
}

uint32_t VulkanApp::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    // 물리 디바이스가 지원하는 메모리 속성을 쿼리합니다.
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(context.getPhysicalDevice(), &memProperties);

    // 지원되는 메모리 타입들을 순회합니다.
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        // 1. typeFilter 비트가 설정되어 있는지 확인 (이 리소스가 사용할 수 있는 메모리 타입인지)
        // 2. 해당 메모리 타입이 우리가 요구하는 모든 속성(properties)을 가지고 있는지 확인
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i; // 적합한 메모리 타입의 인덱스를 반환
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanApp::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    // 1. 일회성 작업을 위한 커맨드 버퍼를 시작합니다.
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    // 2. 이미지 메모리 장벽(Image Memory Barrier)을 설정합니다.
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout; // 이전 레이아웃
    barrier.newLayout = newLayout; // 새로운 레이아웃
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    // 3. 이전/이후 레이아웃에 따라 어떤 파이프라인 단계에서
    //    장벽이 발생해야 하는지, 접근 권한은 어떻게 설정할지 결정합니다.
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    // 4. 커맨드 버퍼에 파이프라인 장벽 명령을 기록합니다.
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    // 5. 커맨드 버퍼 기록을 마치고 GPU에 제출하여 실행합니다.
    endSingleTimeCommands(commandBuffer);
}

void VulkanApp::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    // 1. 일회성 작업을 위한 커맨드 버퍼를 시작합니다.
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    // 2. 복사할 영역(Region)을 정의합니다.
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    // 3. 커맨드 버퍼에 버퍼-이미지 복사 명령을 기록합니다.
    //    이미지의 레이아웃은 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL 상태여야 합니다.
    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    // 4. 커맨드 버퍼 기록을 마치고 GPU에 제출하여 실행합니다.
    endSingleTimeCommands(commandBuffer);
}

// 이 함수들은 graphicsQueue와 commandPool이 멤버 변수로 존재한다고 가정합니다.

VkCommandBuffer VulkanApp::beginSingleTimeCommands() {
    // 1. 커맨드 풀에서 커맨드 버퍼를 하나 할당받습니다.
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = context.getCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(context.getDevice(), &allocInfo, &commandBuffer);

    // 2. 커맨드 버퍼 기록을 시작합니다.
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // 이 버퍼는 한 번만 사용하고 파괴될 것임

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanApp::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    // 1. 커맨드 버퍼 기록을 종료합니다.
    vkEndCommandBuffer(commandBuffer);

    // 2. 제출 정보(Submit Info)를 정의합니다.
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // 3. 그래픽스 큐에 커맨드 버퍼를 제출합니다.
    vkQueueSubmit(context.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    // 4. 작업이 완료될 때까지 CPU가 기다리도록 동기화합니다.
    vkQueueWaitIdle(context.getGraphicsQueue());

    // 5. 사용이 끝난 임시 커맨드 버퍼를 해제합니다.
    vkFreeCommandBuffers(context.getDevice(), context.getCommandPool(), 1, &commandBuffer);
}

// 이 함수는 Vulkan Context나 Base 클래스의 멤버 함수로 만드는 것이 일반적입니다.
// device가 멤버 변수로 존재한다고 가정합니다.

VkImageView VulkanApp::createImageView(VkImage image, VkFormat format) {
    // 1. 이미지 뷰 생성 정보(CreateInfo)를 정의합니다.
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image; // 뷰를 생성할 대상 이미지

    // 2. 이미지의 타입을 지정합니다. (2D, 3D, Cubemap 등)
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format; // 뷰가 이미지를 해석할 픽셀 포맷

    // 3. 컴포넌트 매핑(swizzling)을 기본값으로 설정합니다.
    // (예: R 채널을 G 채널로 보이게 하는 등의 작업을 하지 않음)
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    // 4. 이미지의 어떤 부분을 뷰가 다룰지 정의합니다. (Subresource Range)
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // 컬러 데이터를 다룸
    viewInfo.subresourceRange.baseMipLevel = 0; // 밉맵 레벨 0부터
    viewInfo.subresourceRange.levelCount = 1;   // 1개의 밉맵 레벨을 다룸
    viewInfo.subresourceRange.baseArrayLayer = 0; // 배열의 첫 번째 레이어부터
    viewInfo.subresourceRange.layerCount = 1;   // 1개의 레이어를 다룸

    VkImageView imageView;
    if (vkCreateImageView(context.getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void VulkanApp::createTextureSampler() {
    // 1. 샘플러 생성 정보(CreateInfo)를 정의합니다.
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    // 2. 필터링(Filtering) 설정: 텍스처를 확대(mag)하거나 축소(min)할 때
    //    픽셀 색상을 어떻게 보간할지 결정합니다.
    //    - VK_FILTER_LINEAR: 주변 픽셀과 선형 보간하여 부드럽게 표현합니다. (추천)
    //    - VK_FILTER_NEAREST: 가장 가까운 픽셀 색상을 그대로 사용하여 픽셀 아트처럼 보이게 합니다.
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    // 3. 주소 지정 모드(Address Mode): UV 좌표가 [0, 1] 범위를 벗어날 때
    //    텍스처를 어떻게 처리할지 결정합니다.
    //    - VK_SAMPLER_ADDRESS_MODE_REPEAT: 텍스처를 바둑판식으로 반복합니다. (일반적)
    //    - VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: 거울에 비친 것처럼 반복합니다.
    //    - VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: 가장자리 픽셀 색상을 계속 사용합니다.
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    // 4. 비등방성 필터링(Anisotropic Filtering) 활성화  anisotropia
    //    비스듬한 각도에서 텍스처를 볼 때 흐릿해지는 현상을 줄여 퀄리티를 크게 향상시킵니다.
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(context.getPhysicalDevice(), &properties);

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // GPU가 지원하는 최대값 사용

    // 5. 기타 설정
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // Clamp 모드일 때의 테두리 색상
    samplerInfo.unnormalizedCoordinates = VK_FALSE; // UV 좌표를 [0, 1]로 정규화하여 사용
    samplerInfo.compareEnable = VK_FALSE; // 그림자 맵(PCF) 같은 특수 용도가 아님
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    // 6. 밉맵(Mipmap) 관련 설정
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f; // 지금은 밉맵을 사용하지 않으므로 기본값 설정

    if (vkCreateSampler(context.getDevice(), &samplerInfo, nullptr, &textureSampler_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}