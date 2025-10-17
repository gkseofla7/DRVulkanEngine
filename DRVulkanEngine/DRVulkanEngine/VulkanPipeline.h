#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <array>

// ���� ����
class VulkanContext;
class VulkanSwapChain;

// Vertex ����ü (VulkanApp.h���� �̵�)
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

    // ���� �����ڿ� ���� ������ ���� (RAII ����)
    VulkanPipeline(const VulkanPipeline&) = delete;
    VulkanPipeline& operator=(const VulkanPipeline&) = delete;

    // �̵� �����ڿ� �̵� ���� ������
    VulkanPipeline(VulkanPipeline&& other) noexcept;
    VulkanPipeline& operator=(VulkanPipeline&& other) noexcept;

    // �ʱ�ȭ �� ����
    void initialize(const VulkanContext* vulkanContext, const VulkanSwapChain* vulkanSwapChain);
    void createGraphicsPipeline();
    void cleanup();
    
    // Pipeline ����� (SwapChain ����� ��)
    void recreate();

    // Getter �޼����
    VkPipeline getGraphicsPipeline() const { return graphicsPipeline; }
    VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
    bool isValid() const { return graphicsPipeline != VK_NULL_HANDLE; }

private:
    // ���̴� ���� ���� �޼����
    static std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    
    // Pipeline ���� ��� ���� ���� �޼����
    void createPipelineLayout();
    VkPipelineVertexInputStateCreateInfo createVertexInputState();
    VkPipelineInputAssemblyStateCreateInfo createInputAssemblyState();
    VkPipelineViewportStateCreateInfo createViewportState(VkViewport& viewport, VkRect2D& scissor);
    VkPipelineRasterizationStateCreateInfo createRasterizationState();
    VkPipelineMultisampleStateCreateInfo createMultisampleState();
    VkPipelineColorBlendStateCreateInfo createColorBlendState(VkPipelineColorBlendAttachmentState& colorBlendAttachment);
    VkPipelineRenderingCreateInfo createDynamicRenderingInfo(VkFormat& colorFormat);
};