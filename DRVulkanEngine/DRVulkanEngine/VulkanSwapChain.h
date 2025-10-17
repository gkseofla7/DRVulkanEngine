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
    void cleanup();
    
    // SwapChain ����� (������ ũ�� ���� ��)
    void recreate();

    // Getter �޼����
    VkSwapchainKHR getSwapChain() const { return swapChain; }
    const std::vector<VkImage>& getSwapChainImages() const { return swapChainImages; }
    VkFormat getSwapChainImageFormat() const { return swapChainImageFormat; }
    VkExtent2D getSwapChainExtent() const { return swapChainExtent; }
    const std::vector<VkImageView>& getSwapChainImageViews() const { return swapChainImageViews; }
    
    // ��ƿ��Ƽ �޼����
    uint32_t getImageCount() const { return static_cast<uint32_t>(swapChainImages.size()); }
    bool isValid() const { return swapChain != VK_NULL_HANDLE; }

    // Dynamic Rendering�� ���� ���� �޼����
    VkRenderingAttachmentInfo getColorAttachmentInfo(uint32_t imageIndex) const;
    VkRenderingInfo getRenderingInfo(uint32_t imageIndex) const;

private:
    // ���� ���� �޼����
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};