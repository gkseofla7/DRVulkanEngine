#include "VulkanPipeline.h"
#include "VulkanContext.h"
#include "VulkanSwapChain.h"
#include "PipelineManager.h"
#include "Shader.h"
#include "ShaderManager.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include "DescriptorPool.h"

// Vertex 구조체 구현 (VulkanApp.cpp에서 이동)


VulkanPipeline::~VulkanPipeline() {
    cleanup();
}

VulkanPipeline::VulkanPipeline(VulkanPipeline&& other) noexcept
    : pipelineLayout(other.pipelineLayout)
    , graphicsPipeline(other.graphicsPipeline)
    , context_(other.context_)
    , swapChain_(other.swapChain_) {
    
    // 이동된 객체의 핸들들을 무효화
    other.pipelineLayout = VK_NULL_HANDLE;
    other.graphicsPipeline = VK_NULL_HANDLE;
    other.context_ = nullptr;
    other.swapChain_ = nullptr;
}

VulkanPipeline& VulkanPipeline::operator=(VulkanPipeline&& other) noexcept {
    if (this != &other) {
        cleanup();
        
        pipelineLayout = other.pipelineLayout;
        graphicsPipeline = other.graphicsPipeline;
        context_ = other.context_;
        swapChain_ = other.swapChain_;
        
        other.pipelineLayout = VK_NULL_HANDLE;
        other.graphicsPipeline = VK_NULL_HANDLE;
        other.context_ = nullptr;
        other.swapChain_ = nullptr;
    }
    return *this;
}

void VulkanPipeline::initialize(const VulkanContext* vulkanContext,
    const VulkanSwapChain* vulkanSwapChain,
    DescriptorPool* descriptorPool,
    ShaderManager* shaderMgr,
    const PipelineConfig& config)
{
    context_ = vulkanContext;
    swapChain_ = vulkanSwapChain;
    shaderMgr_ = shaderMgr;
    config_ = config;
    descriptorPool_ = descriptorPool;

    Shader* vertexShader = shaderMgr_->getShader(config_.vertexShaderPath);
    Shader* fragmentShader = shaderMgr_->getShader(config_.fragmentShaderPath);
    createGraphicsPipeline({ vertexShader, fragmentShader });
}

void VulkanPipeline::createGraphicsPipeline(const std::vector<Shader*> shaders){
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    shaderStages.reserve(shaders.size());

    for (const Shader* shader : shaders) {
        shaderStages.push_back(shader->stageInfo_);

    }

    // Pipeline 구성 요소들 생성
    auto vertexInputInfo = createVertexInputState();
    auto inputAssembly = createInputAssemblyState();
    
    VkViewport viewport{};
    VkRect2D scissor{};
    auto viewportState = createViewportState(viewport, scissor);
    
    auto rasterizer = createRasterizationState();
    auto multisampling = createMultisampleState();
    
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    auto colorBlending = createColorBlendState(colorBlendAttachment);

    // Depth Stencil State 추가
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.depthTestEnable = VK_TRUE;
    depthStencilInfo.depthWriteEnable = VK_TRUE;
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.stencilTestEnable = VK_FALSE;

    // Pipeline Layout 생성
    createPipelineLayout(shaders);

    // Dynamic Rendering을 위한 Pipeline Rendering Create Info
    VkFormat colorFormat = swapChain_->getSwapChainImageFormat();
    auto pipelineRenderingCreateInfo = createDynamicRenderingInfo(colorFormat);

    // Graphics Pipeline 생성
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = &pipelineRenderingCreateInfo; // Dynamic Rendering 정보 연결
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencilInfo; // Depth Stencil State 추가
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = VK_NULL_HANDLE; // Dynamic Rendering에서는 null
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(context_->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

void VulkanPipeline::createPipelineLayout(const std::vector<Shader*> shaders) {
    for (const Shader* shader : shaders) {
        for (const auto& [setNumber, bindings] : shader->descriptorSetLayouts_) {
            for (const auto& binding : bindings) {
				const VkDescriptorSetLayoutBinding& bindingInfo = binding.bindingInfo;
                descriptorSetLayoutBindingMap_[setNumber][bindingInfo.binding].resourceName = binding.resourceName;
                descriptorSetLayoutBindingMap_[setNumber][bindingInfo.binding].bindingInfo = bindingInfo;
            }
        }
    }
    VkDevice device = context_->getDevice();
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    descriptorSetLayouts.resize(descriptorSetLayoutBindingMap_.size());

    for (const auto& [setNumber, bindingsMap] : descriptorSetLayoutBindingMap_) {
        // 맵의 바인딩 정보들을 벡터로 변환합니다.
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (const auto& [bindingNumber, bindingInfo] : bindingsMap) {
            bindings.push_back(bindingInfo.bindingInfo);
        }

        descriptorSetLayouts[setNumber] = descriptorPool_->layoutCache_.getLayout(bindings);
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.empty() ? nullptr : descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    if (vkCreatePipelineLayout(context_->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}


void VulkanPipeline::recreate() {
    // 기존 Pipeline 정리
    if (graphicsPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(context_->getDevice(), graphicsPipeline, nullptr);
        graphicsPipeline = VK_NULL_HANDLE;
    }

    if (pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(context_->getDevice(), pipelineLayout, nullptr);
        pipelineLayout = VK_NULL_HANDLE;
    }

    Shader* vertexShader = shaderMgr_->getShader(config_.vertexShaderPath);
    Shader* fragmentShader = shaderMgr_->getShader(config_.fragmentShaderPath);
    createGraphicsPipeline({ vertexShader , fragmentShader });

    std::cout << "Pipeline recreated successfully!" << std::endl;
}

void VulkanPipeline::cleanup() {
    if (!context_ || !context_->getDevice()) {
        return;
    }

    if (graphicsPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(context_->getDevice(), graphicsPipeline, nullptr);
        graphicsPipeline = VK_NULL_HANDLE;
    }

    if (pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(context_->getDevice(), pipelineLayout, nullptr);
        pipelineLayout = VK_NULL_HANDLE;
    }
}

void VulkanPipeline::bindPipeline(VkCommandBuffer commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
}

VkPipelineVertexInputStateCreateInfo VulkanPipeline::createVertexInputState() {
    static auto bindingDescription = Vertex::getBindingDescription();
    static auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    return vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo VulkanPipeline::createInputAssemblyState() {
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    return inputAssembly;
}

VkPipelineViewportStateCreateInfo VulkanPipeline::createViewportState(VkViewport& viewport, VkRect2D& scissor) {
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapChain_->getSwapChainExtent().width;
    viewport.height = (float) swapChain_->getSwapChainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    scissor.offset = {0, 0};
    scissor.extent = swapChain_->getSwapChainExtent();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    return viewportState;
}

VkPipelineRasterizationStateCreateInfo VulkanPipeline::createRasterizationState() {
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    return rasterizer;
}

VkPipelineMultisampleStateCreateInfo VulkanPipeline::createMultisampleState() {
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    return multisampling;
}

VkPipelineColorBlendStateCreateInfo VulkanPipeline::createColorBlendState(VkPipelineColorBlendAttachmentState& colorBlendAttachment) {
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

    return colorBlending;
}

VkPipelineRenderingCreateInfo VulkanPipeline::createDynamicRenderingInfo(VkFormat& colorFormat) {
    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
    pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipelineRenderingCreateInfo.colorAttachmentCount = 1;
    pipelineRenderingCreateInfo.pColorAttachmentFormats = &colorFormat;
    
    // Depth Buffer 포맷 설정
    pipelineRenderingCreateInfo.depthAttachmentFormat = swapChain_->getDepthFormat();
    pipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED; // 스텐실은 사용 안 함

    return pipelineRenderingCreateInfo;
}