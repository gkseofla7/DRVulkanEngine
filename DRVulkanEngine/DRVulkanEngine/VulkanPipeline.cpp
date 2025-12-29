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
#include "DescriptorSet.h"
#include "GlobalData.h"
// Vertex 구조체 구현 (VulkanApp.cpp에서 이동)


VulkanPipeline::~VulkanPipeline() {
    cleanup();
}

VulkanPipeline::VulkanPipeline(VulkanPipeline&& other) noexcept
    : pipelineLayout_(other.pipelineLayout_)
    , graphicsPipeline(other.graphicsPipeline)
    , context_(other.context_)
    , swapChain_(other.swapChain_) {
    
    // 이동된 객체의 핸들들을 무효화
    other.pipelineLayout_ = VK_NULL_HANDLE;
    other.graphicsPipeline = VK_NULL_HANDLE;
    other.context_ = nullptr;
    other.swapChain_ = nullptr;
}

VulkanPipeline& VulkanPipeline::operator=(VulkanPipeline&& other) noexcept {
    if (this != &other) {
        cleanup();
        
        pipelineLayout_ = other.pipelineLayout_;
        graphicsPipeline = other.graphicsPipeline;
        context_ = other.context_;
        swapChain_ = other.swapChain_;
        
        other.pipelineLayout_ = VK_NULL_HANDLE;
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
////////////////////////////////////
    // TODO. 임시 테스트용 코드
    struct SpecializationData {
        VkBool32 useBDA; // 0 (false) 또는 1 (true)
    };

    SpecializationData specData;
    specData.useBDA = USE_BDA_BUFFER? VK_TRUE : VK_FALSE; // 또는 VK_FALSE로 결정

    // 2. 셰이더의 어떤 상수와 매핑할지 정의
    VkSpecializationMapEntry entry{};
    entry.constantID = 0;        // GLSL의 constant_id = 0과 매치
    entry.offset = 0;            // specData 구조체 내의 오프셋
    entry.size = sizeof(VkBool32);

    // 3. 스페셜라이제이션 정보 통합
    VkSpecializationInfo specInfo{};
    specInfo.mapEntryCount = 1;
    specInfo.pMapEntries = &entry;
    specInfo.dataSize = sizeof(SpecializationData);
    specInfo.pData = &specData;

    shaderStages[0].pSpecializationInfo = &specInfo;
////////////////////////////////////

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{}; // 바깥에 선언
    if (config_.useVertexInput) {
        // 기존 로직: 3D 모델용 파이프라인은 정점 입력을 설정합니다.
        vertexInputInfo = createVertexInputState();
    }
    else {
        // 톤 매핑용 파이프라인은 빈 정점 입력 상태를 설정합니다.
        // "버텍스 버퍼에서 아무것도 읽지 않겠다"는 의미입니다.
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;
    }
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
    depthStencilInfo.depthCompareOp = config_.depthCompareOp;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.stencilTestEnable = VK_FALSE;
    
    // Pipeline Layout 생성
    createPipelineLayout(shaders);

    // Dynamic Rendering을 위한 Pipeline Rendering Create Info
    auto pipelineRenderingCreateInfo = createDynamicRenderingInfo();

    // Graphics Pipeline 생성
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = &pipelineRenderingCreateInfo; // Dynamic Rendering 정보 연결
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencilInfo; // Depth Stencil State 추가
    pipelineInfo.layout = pipelineLayout_;
    pipelineInfo.renderPass = VK_NULL_HANDLE; // Dynamic Rendering에서는 null
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pNext = &pipelineRenderingCreateInfo;

    if (vkCreateGraphicsPipelines(context_->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

void VulkanPipeline::createPipelineLayout(const std::vector<Shader*> shaders) {
    for (const Shader* shader : shaders) {
        for (const auto& [setNumber, bindings] : shader->descriptorSetLayouts_) {
            for (const auto& binding : bindings) {
				const VkDescriptorSetLayoutBinding& bindingInfo = binding.bindingInfo;
                if (descriptorSetLayoutBindingMap_.contains(setNumber) && descriptorSetLayoutBindingMap_[setNumber].contains(bindingInfo.binding))
                {
                    descriptorSetLayoutBindingMap_[setNumber][bindingInfo.binding].bindingInfo.stageFlags |= bindingInfo.stageFlags;
                }
                else
                {
                    descriptorSetLayoutBindingMap_[setNumber][bindingInfo.binding].resourceName = binding.resourceName;
                    descriptorSetLayoutBindingMap_[setNumber][bindingInfo.binding].bindingInfo = bindingInfo;
                }
            }
        }
    }
    VkDevice device = context_->getDevice();
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    descriptorSetLayouts.resize(descriptorSetLayoutBindingMap_.size());

    for (const auto& [setNumber, bindingsMap] : descriptorSetLayoutBindingMap_) {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (const auto& [bindingNumber, bindingInfo] : bindingsMap) {
            bindings.push_back(bindingInfo.bindingInfo);
        }

        descriptorSetLayouts[setNumber] = descriptorPool_->layoutCache_.getLayout(bindings);
    }

    std::vector<VkPushConstantRange> totalPushConstantRanges;

    for (const Shader* shader : shaders) {
        for (const auto& range : shader->pushConstantRanges_) {
            auto it = std::find_if(totalPushConstantRanges.begin(), totalPushConstantRanges.end(),
                [&](const VkPushConstantRange& r) {
                    return r.offset == range.offset;
                });

            if (it != totalPushConstantRanges.end()) {
                it->stageFlags |= range.stageFlags;
                it->size = std::max(it->size, range.size);
            }
            else {
                totalPushConstantRanges.push_back(range);
            }
        }
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.empty() ? nullptr : descriptorSetLayouts.data();

    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(totalPushConstantRanges.size());
    pipelineLayoutInfo.pPushConstantRanges = totalPushConstantRanges.empty() ? nullptr : totalPushConstantRanges.data();
    if (vkCreatePipelineLayout(context_->getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}


void VulkanPipeline::recreate() {
    // 기존 Pipeline 정리
    if (graphicsPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(context_->getDevice(), graphicsPipeline, nullptr);
        graphicsPipeline = VK_NULL_HANDLE;
    }

    if (pipelineLayout_ != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(context_->getDevice(), pipelineLayout_, nullptr);
        pipelineLayout_ = VK_NULL_HANDLE;
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

    if (pipelineLayout_ != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(context_->getDevice(), pipelineLayout_, nullptr);
        pipelineLayout_ = VK_NULL_HANDLE;
    }
}
void VulkanPipeline::setDescriptorSets(const std::vector<DescriptorSet>& inDescriptorSet)
{
    descriptorSets_ = inDescriptorSet;
}
void VulkanPipeline::bindPipeline(VkCommandBuffer commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    std::vector< VkDescriptorSet> descriptorSets;
    for (const DescriptorSet& ds : descriptorSets_)
    {
        descriptorSets.push_back(ds.getHandle());
    }
    vkCmdBindDescriptorSets(
        commandBuffer,                      // 현재 기록 중인 커맨드 버퍼
        VK_PIPELINE_BIND_POINT_GRAPHICS,    // 그래픽스 파이프라인에 바인딩
        pipelineLayout_,                     // 파이프라인 생성 시 사용한 레이아웃
        0,                                  // 바인딩할 첫 번째 descriptor set 번호 (set = 0)
        descriptorSets.size(),                                  // 바인딩할 descriptor set의 개수
        descriptorSets.data(),                     // 바인딩할 descriptor set 핸들의 배열 포인터
        0,                                  // 동적 오프셋 개수 (없으면 0)
        nullptr                             // 동적 오프셋 배열 포인터 (없으면 nullptr)
    );
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
    rasterizer.cullMode = config_.cullMode;
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
    colorBlendAttachment.blendEnable = config_.blendEnable;
    colorBlendAttachment.srcColorBlendFactor = config_.srcColorBlendFactor;
    colorBlendAttachment.dstColorBlendFactor = config_.dstColorBlendFactor;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // 일반적인 알파 블렌딩 설정
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;


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

VkPipelineRenderingCreateInfo VulkanPipeline::createDynamicRenderingInfo( ) {
    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
    pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipelineRenderingCreateInfo.colorAttachmentCount = 1;
    pipelineRenderingCreateInfo.pColorAttachmentFormats = &config_.colorAttachmentFormat;
    
    // Depth Buffer 포맷 설정
    pipelineRenderingCreateInfo.depthAttachmentFormat = config_.depthAttachmentFormat;
    pipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED; // 스텐실은 사용 안 함

    return pipelineRenderingCreateInfo;
}