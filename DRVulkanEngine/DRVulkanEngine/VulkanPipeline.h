#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <array>
#include <map>
#include "Vertex.h"
#include "PipelineConfig.h"
#include "Shader.h"

class VulkanContext;
class VulkanSwapChain;
class ShaderManager;
class Shader;
class DescriptorPool;

class VulkanPipeline {
public:
    VulkanPipeline() = default;
    ~VulkanPipeline();

    // 복사 생성자와 대입 연산자 삭제 (RAII 패턴)
    VulkanPipeline(const VulkanPipeline&) = delete;
    VulkanPipeline& operator=(const VulkanPipeline&) = delete;

    // 이동 생성자와 이동 대입 연산자
    VulkanPipeline(VulkanPipeline&& other) noexcept;
    VulkanPipeline& operator=(VulkanPipeline&& other) noexcept;

    void initialize(const VulkanContext* vulkanContext,
        const VulkanSwapChain* vulkanSwapChain,
        DescriptorPool* descriptorPool,
        ShaderManager* shaderMgr,
        const PipelineConfig& config);

    void cleanup();
	void bindPipeline(VkCommandBuffer commandBuffer);


    // Pipeline 재생성 (SwapChain 재생성 시)
    void recreate();

    // Getter 메서드들
    VkPipeline getGraphicsPipeline() const { return graphicsPipeline; }
    VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
    const std::map<uint32_t, std::map<uint32_t, LayoutBindingInfo>>& GetDescriptorSetLayoutBindingMap() { return descriptorSetLayoutBindingMap_; };

    bool isValid() const { return graphicsPipeline != VK_NULL_HANDLE; }

private:
    void createGraphicsPipeline(const std::vector<Shader*> shaders);
    void createPipelineLayout(const std::vector<Shader*> shaders);
private:
    // 셰이더 관련 헬퍼 메서드들
    static std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    
    // Pipeline 구성 요소 생성 헬퍼 메서드들
    
    VkPipelineVertexInputStateCreateInfo createVertexInputState();
    VkPipelineInputAssemblyStateCreateInfo createInputAssemblyState();
    VkPipelineViewportStateCreateInfo createViewportState(VkViewport& viewport, VkRect2D& scissor);
    VkPipelineRasterizationStateCreateInfo createRasterizationState();
    VkPipelineMultisampleStateCreateInfo createMultisampleState();
    VkPipelineColorBlendStateCreateInfo createColorBlendState(VkPipelineColorBlendAttachmentState& colorBlendAttachment);
    VkPipelineRenderingCreateInfo createDynamicRenderingInfo(VkFormat& colorFormat);
private:
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

    const VulkanContext* context_;
    const VulkanSwapChain* swapChain_;
    ShaderManager* shaderMgr_;
    DescriptorPool* descriptorPool_;

    PipelineConfig config_;
    std::map<uint32_t, std::map<uint32_t, LayoutBindingInfo>> descriptorSetLayoutBindingMap_;
};