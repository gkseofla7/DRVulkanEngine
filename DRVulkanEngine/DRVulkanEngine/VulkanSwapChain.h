#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

// GLFW ���� ����
struct GLFWwindow;
class VulkanContext;

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanSwapChain {
private:
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    
    // Depth Buffer ���� ��� �߰�
    VkImage depthImage = VK_NULL_HANDLE;
    VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;
    VkFormat depthFormat;

    const VulkanContext* context;
    GLFWwindow* window;

public:
    VulkanSwapChain() = default;
    ~VulkanSwapChain();

    // ���� �����ڿ� ���� ������ ���� (RAII ����)
    VulkanSwapChain(const VulkanSwapChain&) = delete;
    VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;

    // �̵� �����ڿ� �̵� ���� ������
    VulkanSwapChain(VulkanSwapChain&& other) noexcept;
    VulkanSwapChain& operator=(VulkanSwapChain&& other) noexcept;

    // �ʱ�ȭ �޼���� (RenderPass ������ ����)
    void initialize(const VulkanContext* vulkanContext, GLFWwindow* glfwWindow);
    void createSwapChain();
    void createImageViews();
    void createDepthResources(); // Depth Buffer ���� �޼��� �߰�
    void cleanup();
    
    // SwapChain ����� (������ ũ�� ���� ��)
    void recreate();

    // Getter �޼����
    VkSwapchainKHR getSwapChain() const { return swapChain; }
    const std::vector<VkImage>& getSwapChainImages() const { return swapChainImages; }
    VkFormat getSwapChainImageFormat() const { return swapChainImageFormat; }
    VkExtent2D getSwapChainExtent() const { return swapChainExtent; }
    const std::vector<VkImageView>& getSwapChainImageViews() const { return swapChainImageViews; }
    
    // Depth Buffer Getter �޼���� �߰�
    VkImageView getDepthImageView() const { return depthImageView; }
    VkFormat getDepthFormat() const { return depthFormat; }
    
    // ��ƿ��Ƽ �޼����
    uint32_t getImageCount() const { return static_cast<uint32_t>(swapChainImages.size()); }
    bool isValid() const { return swapChain != VK_NULL_HANDLE; }

    // Dynamic Rendering�� ���� ���� �޼����
    VkRenderingAttachmentInfo getColorAttachmentInfo(uint32_t imageIndex) const;
    VkRenderingAttachmentInfo getDepthAttachmentInfo() const; // Depth Attachment ���� �߰�
    VkRenderingInfo getRenderingInfo(uint32_t imageIndex) const;

private:
    // ���� ���� �޼����
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    
    // Depth Buffer ���� ���� �޼���� �߰�
    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    bool hasStencilComponent(VkFormat format);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
};