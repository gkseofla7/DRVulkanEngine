#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <array>

// 전방 선언
class VulkanContext;
class VulkanSwapChain;

// Vertex 구조체 (VulkanApp.h에서 이동)
struct Vertex {
    float pos[2];
    float color[3];

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};

class VulkanPipeline {
private:
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    
    const VulkanContext* context;
    const VulkanSwapChain* swapChain;

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
    void initialize(const VulkanContext* vulkanContext, const VulkanSwapChain* vulkanSwapChain);
    void createGraphicsPipeline();
    void cleanup();
    
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