#include "VulkanApp.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <set>
#include <algorithm>
#include "Camera.h"

// 삼각형 정점 데이터
// 정육면체를 구성하는 8개의 고유한 정점(Vertex) 데이터
// 각 면이 고유한 색상을 갖도록 정의된 24개의 정점 데이터
const std::vector<Vertex> vertices = {
    // 앞면 (빨강) - 정점 인덱스 0-3
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // 0
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // 1
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // 2
    {{-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // 3

    // 뒷면 (파랑) - 정점 인덱스 4-7
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}, // 4
    {{ 0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}, // 5
    {{ 0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}, // 6
    {{-0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}, // 7

    // 왼쪽 면 (초록) - 정점 인덱스 8-11
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}}, // 8
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}}, // 9
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}}, // 10
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}}, // 11

    // 오른쪽 면 (노랑) - 정점 인덱스 12-15
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}}, // 12
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}}, // 13
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}}, // 14
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}}, // 15

    // 윗면 (시안) - 정점 인덱스 16-19
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}}, // 16
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}}, // 17
    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}}, // 18
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}}, // 19

    // 아랫면 (마젠타) - 정점 인덱스 20-23
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}}, // 20
    {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}}, // 21
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}}, // 22
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}}  // 23
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
// Vertex 구조체 구현
VkVertexInputBindingDescription Vertex::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> Vertex::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
    
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    return attributeDescriptions;
}

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
    swapChain.initialize(&context, window); // RenderPass 없이 SwapChain 초기화

    createUniformBuffers();
    createDescriptorSetLayout();
    createDescriptorPool();
    createDescriptorSets();
    createGraphicsPipeline();
    createVertexBuffer();
    createIndexBuffer();
    createCommandBuffers();
    createSyncObjects();
}

void VulkanApp::createGraphicsPipeline() {
    auto vertShaderCode = readFile("shaders/shader.vert.spv");
    auto fragShaderCode = readFile("shaders/shader.frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapChain.getSwapChainExtent().width;
    viewport.height = (float) swapChain.getSwapChainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChain.getSwapChainExtent();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.depthTestEnable = VK_TRUE;
    depthStencilInfo.depthWriteEnable = VK_TRUE;
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(context.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    // Dynamic Rendering을 위한 Pipeline Rendering Create Info
    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
    pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipelineRenderingCreateInfo.colorAttachmentCount = 1;
    VkFormat colorAttachmentFormat = swapChain.getSwapChainImageFormat();
    pipelineRenderingCreateInfo.pColorAttachmentFormats = &colorAttachmentFormat;

    pipelineRenderingCreateInfo.depthAttachmentFormat = swapChain.getDepthFormat(); // 예시: context에서 깊이 포맷 가져오기
    pipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED; // 스텐실은 사용 안 함

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = &pipelineRenderingCreateInfo; // Dynamic Rendering 정보 연결
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencilInfo;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = VK_NULL_HANDLE; // Dynamic Rendering에서는 null
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(context.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(context.getDevice(), fragShaderModule, nullptr);
    vkDestroyShaderModule(context.getDevice(), vertShaderModule, nullptr);

    std::cout << "Graphics pipeline created with Dynamic Rendering!" << std::endl;
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

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    vkCmdBindDescriptorSets(commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
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

    vkDestroyPipeline(context.getDevice(), graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(context.getDevice(), pipelineLayout, nullptr);

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
    // 0번 바인딩에 UBO가 하나 있다는 정보를 정의합니다.
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0; // layout(binding = 0) in shader
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1; // 배열이 아니므로 1개
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // 버텍스 셰이더에서 사용
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(context.getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void VulkanApp::createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(swapChain.getImageCount());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
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

    // 할당된 각 디스크립터 셋에 실제 Uniform Buffer 정보를 기록(업데이트)합니다.
    for (size_t i = 0; i < swapChain.getImageCount(); i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(context.getDevice(), 1, &descriptorWrite, 0, nullptr);
    }
}