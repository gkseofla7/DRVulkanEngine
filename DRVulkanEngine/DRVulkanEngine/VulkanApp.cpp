#include "VulkanApp.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <set>
#include <algorithm>
#include "Camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
// �ﰢ�� ���� ������
// ������ü�� �����ϴ� 8���� ������ ����(Vertex) ������
// �� ���� ������ ������ ������ ���ǵ� 24���� ���� ������
const std::vector<Vertex> vertices = {
    // �ո� (����) - ���� �ε��� 0-3
    // pos                      color                  texCoord
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // 0: Bottom-left
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // 1: Bottom-right
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // 2: Top-right
    {{-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 3: Top-left

    // �޸� (�Ķ�) - ���� �ε��� 4-7
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 4: Bottom-right
    {{ 0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 5: Bottom-left
    {{ 0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // 6: Top-left
    {{-0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // 7: Top-right

    // ���� �� (�ʷ�) - ���� �ε��� 8-11
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // 8: Bottom-right
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // 9: Bottom-left
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // 10: Top-left
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // 11: Top-right

    // ������ �� (���) - ���� �ε��� 12-15
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // 12: Bottom-left
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // 13: Top-left
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // 14: Top-right
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // 15: Bottom-right

    // ���� (�þ�) - ���� �ε��� 16-19
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // 16: Bottom-left
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}, // 17: Bottom-right
    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}, // 18: Top-right
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // 19: Top-left

    // �Ʒ��� (����Ÿ) - ���� �ε��� 20-23
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // 20: Top-left
    {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 21: Bottom-left
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 22: Bottom-right
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}  // 23: Top-right
};

// �� 'vertices' �迭�� �ε����� ����Ͽ� 6���� ��(�ﰢ�� 12��)�� �׸��ϴ�.
// �ּ�: ���ε� ����(Winding Order)�� �ݽð� ����(Counter-Clockwise)�� �������� �մϴ�.
// �̴� �޸� �ø�(Back-face Culling)�� �ùٸ��� ó���ϱ� �����Դϴ�.
const std::vector<uint16_t> indices = {
    // �ո� (CCW)
    0, 1, 2,   0, 2, 3,
    // �޸� (CCW)
    4, 5, 6,   6, 7, 4,
    // ���� �� (CCW)
    8, 9, 10,  10, 11, 8,

    // ������ �� (CCW)
    12, 15, 14,  14, 13, 12,

    // ���� (CCW)
    16, 17, 18,  18, 19, 16,

    // �Ʒ��� (CCW)
    20, 23, 22,  22, 21, 20
};
VulkanApp::VulkanApp()
    :camera(std::make_unique<Camera>(glm::vec3{ 3.0f, 3.0f, -3.0f },
        glm::vec3{ 0.0f, 0.0f, 0.0f },
        glm::vec3{ 0.0f, 1.0f, 0.0f }))
{

}

VulkanApp::~VulkanApp() = default;

// VulkanApp Ŭ���� ����
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
    swapChain.initialize(&context, window); // RenderPass ���� SwapChain �ʱ�ȭ
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
    // 1. ������ ũ�⸦ 'indices' ���͸� �������� ����մϴ�.
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    // 2. ������ �뵵�� VK_BUFFER_USAGE_INDEX_BUFFER_BIT�� �����մϴ�.
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // 'indexBuffer' �ڵ鿡 ���۸� �����մϴ�.
    if (vkCreateBuffer(context.getDevice(), &bufferInfo, nullptr, &indexBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create index buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context.getDevice(), indexBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = context.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // 'indexBufferMemory' �ڵ鿡 �޸𸮸� �Ҵ��մϴ�.
    if (vkAllocateMemory(context.getDevice(), &allocInfo, nullptr, &indexBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate index buffer memory!");
    }

    // ������ ���ۿ� �޸𸮸� ����(���ε�)�մϴ�.
    vkBindBufferMemory(context.getDevice(), indexBuffer, indexBufferMemory, 0);

    // ���� �����͸� ���ۿ� �����մϴ�.
    void* data;
    vkMapMemory(context.getDevice(), indexBufferMemory, 0, bufferSize, 0, &data);
    // 3. ���� �����͸� 'vertices' ��� 'indices' ���Ϳ��� �����ɴϴ�.
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

    // �̹��� ���̾ƿ� ��ȯ (UNDEFINED -> COLOR_ATTACHMENT_OPTIMAL)
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

    // Dynamic Rendering ����
    auto renderingInfo = swapChain.getRenderingInfo(imageIndex);
    vkCmdBeginRendering(commandBuffer, &renderingInfo);

	pipeline.bindPipeline(commandBuffer);

    vkCmdBindDescriptorSets(commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline.getPipelineLayout(),
        0, // firstSet: ��ũ���� �� ���̾ƿ� �迭�� ���� �ε���
        1, // descriptorSetCount: ���ε��� ��ũ���� ���� ����
        &descriptorSets[currentFrame], // pDescriptorSets: ���ε��� ��ũ���� ���� ������
        0, // dynamicOffsetCount
        nullptr); // pDynamicOffsets
    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);


    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    vkCmdEndRendering(commandBuffer);

    // �̹��� ���̾ƿ� ��ȯ (COLOR_ATTACHMENT_OPTIMAL -> PRESENT_SRC)
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

    // SwapChain�� VulkanContext�� �ڵ����� ������ (RAII)

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

    // ����ü�� �̹��� ������ŭ ���ۿ� �޸� �ڵ��� ���� ������ �̸� Ȯ���մϴ�.
    uniformBuffers.resize(swapChainImageCount);
    uniformBuffersMemory.resize(swapChainImageCount);
    uniformBuffersMapped.resize(swapChainImageCount);

    // �� ����ü�� �̹����� ���� ������ ���ϴ�.
    for (size_t i = 0; i < swapChainImageCount; i++) {
        // ���� ����
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(context.getDevice(), &bufferInfo, nullptr, &uniformBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create uniform buffer!");
        }

        // �޸� �䱸���� Ȯ�� �� �Ҵ�
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(context.getDevice(), uniformBuffers[i], &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        //CPU���� ���� �����ϰ�(HOST_VISIBLE), ���� ��� GPU�� �ݿ��Ǵ�(HOST_COHERENT) �Ӽ����� �޸𸮸� ã���ϴ�.
        allocInfo.memoryTypeIndex = context.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(context.getDevice(), &allocInfo, nullptr, &uniformBuffersMemory[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate uniform buffer memory!");
        }

        // ���ۿ� �޸� ���ε�
        vkBindBufferMemory(context.getDevice(), uniformBuffers[i], uniformBuffersMemory[i], 0);

        //�ٽ�: �޸𸮸� ���������� �����ϰ�, �� �ּҸ� uniformBuffersMapped�� �����մϴ�.
        // unmap�� ȣ������ �����Ƿ�, �� �ּҴ� ���α׷� ���� ������ ��� ��ȿ�մϴ�.
        vkMapMemory(context.getDevice(), uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void VulkanApp::updateUniformBuffer(uint32_t currentImage) {
    UniformBufferObject ubo{};

    // 1. ī�޶�κ��� View ��Ʈ���� ��������
    ubo.view = camera->getViewMatrix();

    // 2. ī�޶�κ��� Projection ��Ʈ���� ��������
    // �þ߰�: 45��, ȭ���: ����ü�� ����, Near/Far: 0.1/10.0
    ubo.proj = camera->getProjectionMatrix(45.0f,
        swapChain.getSwapChainExtent().width / (float)swapChain.getSwapChainExtent().height,
        0.1f, 10.0f);

    // 3. UBO �����͸� GPU �޸𸮿� ����
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void VulkanApp::createDescriptorSetLayout() {
    // 1. Uniform Buffer Object(UBO) ���ε� ���� (binding = 0)
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // ���ؽ� ���̴����� ���
    uboLayoutBinding.pImmutableSamplers = nullptr;

    // 2. Combined Image Sampler ���ε� ���� (binding = 1)
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1; // ���̴��� layout(binding = 1)�� ��ġ
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // �����׸�Ʈ ���̴����� ���

    // 3. ������ ������ ���ε����� �迭�� ����ϴ�.
    std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding, samplerLayoutBinding };

    // 4. ���̾ƿ� ���� ���� ������Ʈ
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size()); // ���ε� ������ 2�� ����
    layoutInfo.pBindings = bindings.data(); // ���ε� �迭�� �����͸� ����

    if (vkCreateDescriptorSetLayout(context.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}
void VulkanApp::createDescriptorPool() {
    std::vector<VkDescriptorPoolSize> poolSizes(2);

    // UBO�� ���� Ǯ ũ��
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

    // �Ҵ�� �� ��ũ���� �¿� ���� ���ҽ� ������ ���(������Ʈ)�մϴ�.
    for (size_t i = 0; i < swapChain.getImageCount(); i++) {
        // 1. UBO ���� ���� (binding = 0)
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        // 2. �ؽ�ó �̹��� �� ���÷� ���� ���� (binding = 1) 
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // ���̴��� �б� ���� ���̾ƿ�
        imageInfo.imageView = textureView_; // �����ص� �ؽ�ó �̹��� ��
        imageInfo.sampler = textureSampler_;     // �����ص� �ؽ�ó ���÷�

        // 3. �� ���� ���� �۾��� �迭�� ����ϴ�.
        std::vector<VkWriteDescriptorSet> descriptorWrites(2);

        // UBO ���� �۾� ����
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        // �ؽ�ó ���÷� ���� �۾� ���� (�߰��� �κ�)
        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1; // ���̴��� layout(binding = 1)�� ��ġ
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo; // ���۰� �ƴ� �̹��� ������ ����

        // 4. �� ���� ���� �۾��� �ѹ��� ������Ʈ�մϴ�.
        vkUpdateDescriptorSets(context.getDevice(),
            static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(),
            0, nullptr);
    }
}

void VulkanApp::createTextureImage() {
    int texWidth, texHeight, texChannels;
    // stbi_load �Լ��� �̹��� ������ �ҷ��ɴϴ�.
    // STBI_rgb_alpha �÷��״� �̹����� ������ RGBA(4ä��) �������� �ε��մϴ�.
    stbi_uc* pixels = stbi_load("../assets/images/minion.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    // CPU���� ���� �����ϰ�(HOST_VISIBLE), ������ ����(TRANSFER_SRC)�� �� ���۸� �����մϴ�.
    createBuffer(imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    // Staging Buffer�� �޸𸮸� �����Ͽ� CPU�� �ȼ� �����͸� �����մϴ�.
    void* data;
    vkMapMemory(context.getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(context.getDevice(), stagingBufferMemory);

    // ���� �ȼ� �����ʹ� ���� CPU �޸𸮿��� �����ص� �˴ϴ�.
    stbi_image_free(pixels);


// �̹��� ����
    createImage(texWidth, texHeight,
        VK_FORMAT_R8G8B8A8_SRGB, // SRGB ������ ������ �� �ڿ������� ǥ���մϴ�.
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, // ���� ��� + ���̴� ���ø���
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, // �ְ��� ������ ���� GPU ���� �޸𸮿� ����
        texture_,
        textureMemory_);

    // ���̾ƿ� ���� �� ������ ����
    // (1) UNDEFINED -> TRANSFER_DST_OPTIMAL (���� �ޱ� ���� ���̾ƿ����� ����)
    transitionImageLayout(texture_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // (2) Staging Buffer���� ���� �̹����� �ȼ� ������ ����
    copyBufferToImage(stagingBuffer, texture_, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    // (3) TRANSFER_DST_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL (���̴��� �б� ���� ���̾ƿ����� ����)
    transitionImageLayout(texture_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // ���簡 �������Ƿ� Staging Buffer�� �ı��մϴ�.
    vkDestroyBuffer(context.getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(context.getDevice(), stagingBufferMemory, nullptr);

    // �̹��� �� ����
    // ���̴��� VkImage�� �ƴ� VkImageView�� ���� �̹����� �����մϴ�.
    textureView_ = createImageView(texture_, VK_FORMAT_R8G8B8A8_SRGB);
}

// �� �Լ����� ���� Vulkan Context�� Base Ŭ������ ��� �Լ��� ����ϴ�.
// device�� physicalDevice�� ��� ������ �����Ѵٰ� �����մϴ�.

void VulkanApp::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    // 1. ���� ���� ����(CreateInfo) ����
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size; // ������ ũ�� (����Ʈ ����)
    bufferInfo.usage = usage; // ������ �뵵 (��: ���� ����, ���� ���� ��)
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // ���� ť������ ��� (������ ����)

    // 2. ���� �ڵ� ����
    if (vkCreateBuffer(context.getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    // 3. ���ۿ� �ʿ��� �޸� �䱸���� ����
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context.getDevice(), buffer, &memRequirements);

    // 4. �޸� �Ҵ� ����(AllocateInfo) ����
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size; // �ʿ��� �޸� ũ��
    // �䱸���װ� �Ӽ��� �����ϴ� ������ �޸� Ÿ���� ã���ϴ�.
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    // 5. GPU �޸� �Ҵ�
    if (vkAllocateMemory(context.getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    // 6. �Ҵ�� �޸𸮸� ���ۿ� ���ε�(����)
    vkBindBufferMemory(context.getDevice(), buffer, bufferMemory, 0);
}

void VulkanApp::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    // 1. �̹��� ���� ����(CreateInfo) ����
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1; // 2D �ؽ�ó�̹Ƿ� depth�� 1
    imageInfo.mipLevels = 1; // ������ �Ӹ��� ������� ����
    imageInfo.arrayLayers = 1; // �迭 �ؽ�ó�� �ƴ�
    imageInfo.format = format; // �̹����� �ȼ� ���� (��: RGBA)
    imageInfo.tiling = tiling; // �ؼ� ��ġ ��� (Optimal�� GPU�� ����)
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // �ʱ� ���̾ƿ��� �Ű� ���� ����
    imageInfo.usage = usage; // �̹����� �뵵 (��: �ؽ�ó ���ø�, ���� ���)
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // ��Ƽ���ø� ��� �� ��

    // 2. �̹��� �ڵ� ����
    if (vkCreateImage(context.getDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    // 3. �̹����� �ʿ��� �޸� �䱸���� ����
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(context.getDevice(), image, &memRequirements);

    // 4. �޸� �Ҵ� ����(AllocateInfo) ����
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    // 5. GPU �޸� �Ҵ�
    if (vkAllocateMemory(context.getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    // 6. �Ҵ�� �޸𸮸� �̹����� ���ε�(����)
    vkBindImageMemory(context.getDevice(), image, imageMemory, 0);
}

uint32_t VulkanApp::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    // ���� ����̽��� �����ϴ� �޸� �Ӽ��� �����մϴ�.
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(context.getPhysicalDevice(), &memProperties);

    // �����Ǵ� �޸� Ÿ�Ե��� ��ȸ�մϴ�.
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        // 1. typeFilter ��Ʈ�� �����Ǿ� �ִ��� Ȯ�� (�� ���ҽ��� ����� �� �ִ� �޸� Ÿ������)
        // 2. �ش� �޸� Ÿ���� �츮�� �䱸�ϴ� ��� �Ӽ�(properties)�� ������ �ִ��� Ȯ��
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i; // ������ �޸� Ÿ���� �ε����� ��ȯ
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanApp::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    // 1. ��ȸ�� �۾��� ���� Ŀ�ǵ� ���۸� �����մϴ�.
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    // 2. �̹��� �޸� �庮(Image Memory Barrier)�� �����մϴ�.
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout; // ���� ���̾ƿ�
    barrier.newLayout = newLayout; // ���ο� ���̾ƿ�
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

    // 3. ����/���� ���̾ƿ��� ���� � ���������� �ܰ迡��
    //    �庮�� �߻��ؾ� �ϴ���, ���� ������ ��� �������� �����մϴ�.
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

    // 4. Ŀ�ǵ� ���ۿ� ���������� �庮 ����� ����մϴ�.
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    // 5. Ŀ�ǵ� ���� ����� ��ġ�� GPU�� �����Ͽ� �����մϴ�.
    endSingleTimeCommands(commandBuffer);
}

void VulkanApp::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    // 1. ��ȸ�� �۾��� ���� Ŀ�ǵ� ���۸� �����մϴ�.
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    // 2. ������ ����(Region)�� �����մϴ�.
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

    // 3. Ŀ�ǵ� ���ۿ� ����-�̹��� ���� ����� ����մϴ�.
    //    �̹����� ���̾ƿ��� VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ���¿��� �մϴ�.
    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    // 4. Ŀ�ǵ� ���� ����� ��ġ�� GPU�� �����Ͽ� �����մϴ�.
    endSingleTimeCommands(commandBuffer);
}

// �� �Լ����� graphicsQueue�� commandPool�� ��� ������ �����Ѵٰ� �����մϴ�.

VkCommandBuffer VulkanApp::beginSingleTimeCommands() {
    // 1. Ŀ�ǵ� Ǯ���� Ŀ�ǵ� ���۸� �ϳ� �Ҵ�޽��ϴ�.
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = context.getCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(context.getDevice(), &allocInfo, &commandBuffer);

    // 2. Ŀ�ǵ� ���� ����� �����մϴ�.
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // �� ���۴� �� ���� ����ϰ� �ı��� ����

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanApp::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    // 1. Ŀ�ǵ� ���� ����� �����մϴ�.
    vkEndCommandBuffer(commandBuffer);

    // 2. ���� ����(Submit Info)�� �����մϴ�.
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // 3. �׷��Ƚ� ť�� Ŀ�ǵ� ���۸� �����մϴ�.
    vkQueueSubmit(context.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    // 4. �۾��� �Ϸ�� ������ CPU�� ��ٸ����� ����ȭ�մϴ�.
    vkQueueWaitIdle(context.getGraphicsQueue());

    // 5. ����� ���� �ӽ� Ŀ�ǵ� ���۸� �����մϴ�.
    vkFreeCommandBuffers(context.getDevice(), context.getCommandPool(), 1, &commandBuffer);
}

// �� �Լ��� Vulkan Context�� Base Ŭ������ ��� �Լ��� ����� ���� �Ϲ����Դϴ�.
// device�� ��� ������ �����Ѵٰ� �����մϴ�.

VkImageView VulkanApp::createImageView(VkImage image, VkFormat format) {
    // 1. �̹��� �� ���� ����(CreateInfo)�� �����մϴ�.
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image; // �並 ������ ��� �̹���

    // 2. �̹����� Ÿ���� �����մϴ�. (2D, 3D, Cubemap ��)
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format; // �䰡 �̹����� �ؼ��� �ȼ� ����

    // 3. ������Ʈ ����(swizzling)�� �⺻������ �����մϴ�.
    // (��: R ä���� G ä�η� ���̰� �ϴ� ���� �۾��� ���� ����)
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    // 4. �̹����� � �κ��� �䰡 �ٷ��� �����մϴ�. (Subresource Range)
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // �÷� �����͸� �ٷ�
    viewInfo.subresourceRange.baseMipLevel = 0; // �Ӹ� ���� 0����
    viewInfo.subresourceRange.levelCount = 1;   // 1���� �Ӹ� ������ �ٷ�
    viewInfo.subresourceRange.baseArrayLayer = 0; // �迭�� ù ��° ���̾����
    viewInfo.subresourceRange.layerCount = 1;   // 1���� ���̾ �ٷ�

    VkImageView imageView;
    if (vkCreateImageView(context.getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void VulkanApp::createTextureSampler() {
    // 1. ���÷� ���� ����(CreateInfo)�� �����մϴ�.
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    // 2. ���͸�(Filtering) ����: �ؽ�ó�� Ȯ��(mag)�ϰų� ���(min)�� ��
    //    �ȼ� ������ ��� �������� �����մϴ�.
    //    - VK_FILTER_LINEAR: �ֺ� �ȼ��� ���� �����Ͽ� �ε巴�� ǥ���մϴ�. (��õ)
    //    - VK_FILTER_NEAREST: ���� ����� �ȼ� ������ �״�� ����Ͽ� �ȼ� ��Ʈó�� ���̰� �մϴ�.
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    // 3. �ּ� ���� ���(Address Mode): UV ��ǥ�� [0, 1] ������ ��� ��
    //    �ؽ�ó�� ��� ó������ �����մϴ�.
    //    - VK_SAMPLER_ADDRESS_MODE_REPEAT: �ؽ�ó�� �ٵ��ǽ����� �ݺ��մϴ�. (�Ϲ���)
    //    - VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: �ſ￡ ��ģ ��ó�� �ݺ��մϴ�.
    //    - VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: �����ڸ� �ȼ� ������ ��� ����մϴ�.
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    // 4. ���漺 ���͸�(Anisotropic Filtering) Ȱ��ȭ  anisotropia
    //    �񽺵��� �������� �ؽ�ó�� �� �� �帴������ ������ �ٿ� ����Ƽ�� ũ�� ����ŵ�ϴ�.
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(context.getPhysicalDevice(), &properties);

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // GPU�� �����ϴ� �ִ밪 ���

    // 5. ��Ÿ ����
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // Clamp ����� ���� �׵θ� ����
    samplerInfo.unnormalizedCoordinates = VK_FALSE; // UV ��ǥ�� [0, 1]�� ����ȭ�Ͽ� ���
    samplerInfo.compareEnable = VK_FALSE; // �׸��� ��(PCF) ���� Ư�� �뵵�� �ƴ�
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    // 6. �Ӹ�(Mipmap) ���� ����
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f; // ������ �Ӹ��� ������� �����Ƿ� �⺻�� ����

    if (vkCreateSampler(context.getDevice(), &samplerInfo, nullptr, &textureSampler_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}