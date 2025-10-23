#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <array>
#include "Vertex.h"
class VulkanContext;
class VulkanSwapChain;

class VulkanPipeline {
private:
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    
    const VulkanContext* context;
    const VulkanSwapChain* swapChain;
    std::vector<VkDescriptorSetLayout*> descriptorSetLayouts_;
public:
    VulkanPipeline() = default;
    ~VulkanPipeline();

    // 복사 생성자와 대입 연산자 삭제 (RAII 패턴)
    VulkanPipeline(const VulkanPipeline&) = delete;
    VulkanPipeline& operator=(const VulkanPipeline&) = delete;

    // 이동 생성자와 이동 대입 연산자
    VulkanPipeline(VulkanPipeline&& other) noexcept;
    VulkanPipeline& operator=(VulkanPipeline&& other) noexcept;

    // 초기화 및 정리
    void initialize(const VulkanContext* vulkanContext, const VulkanSwapChain* vulkanSwapChain, const std::vector<VkDescriptorSetLayout*> inDescriptorSetLayouts);
    void createGraphicsPipeline();
    void cleanup();
	void bindPipeline(VkCommandBuffer commandBuffer);


    // Pipeline 재생성 (SwapChain 재생성 시)
    void recreate();

    // Getter 메서드들
    VkPipeline getGraphicsPipeline() const { return graphicsPipeline; }
    VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
    bool isValid() const { return graphicsPipeline != VK_NULL_HANDLE; }

private:
    // 셰이더 관련 헬퍼 메서드들
    static std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    
    // Pipeline 구성 요소 생성 헬퍼 메서드들
    void createPipelineLayout();
    VkPipelineVertexInputStateCreateInfo createVertexInputState();
    VkPipelineInputAssemblyStateCreateInfo createInputAssemblyState();
    VkPipelineViewportStateCreateInfo createViewportState(VkViewport& viewport, VkRect2D& scissor);
    VkPipelineRasterizationStateCreateInfo createRasterizationState();
    VkPipelineMultisampleStateCreateInfo createMultisampleState();
    VkPipelineColorBlendStateCreateInfo createColorBlendState(VkPipelineColorBlendAttachmentState& colorBlendAttachment);
    VkPipelineRenderingCreateInfo createDynamicRenderingInfo(VkFormat& colorFormat);
};