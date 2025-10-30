#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

// GLFW 전방 선언
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
    
    // Depth Buffer 관련 멤버 추가
    VkImage depthImage = VK_NULL_HANDLE;
    VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;
    VkFormat depthFormat;

    const VulkanContext* context;
    GLFWwindow* window;

public:
    VulkanSwapChain() = default;
    ~VulkanSwapChain();

    // 복사 생성자와 대입 연산자 삭제 (RAII 패턴)
    VulkanSwapChain(const VulkanSwapChain&) = delete;
    VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;

    // 이동 생성자와 이동 대입 연산자
    VulkanSwapChain(VulkanSwapChain&& other) noexcept;
    VulkanSwapChain& operator=(VulkanSwapChain&& other) noexcept;

    // 초기화 메서드들 (RenderPass 의존성 제거)
    void initialize(const VulkanContext* vulkanContext, GLFWwindow* glfwWindow);
    void createSwapChain();
    void createImageViews();
    void createDepthResources(); // Depth Buffer 생성 메서드 추가
    void cleanup();
    
    // SwapChain 재생성 (윈도우 크기 변경 등)
    void recreate();
    // Getter 메서드들
    VkSwapchainKHR getSwapChain() const { return swapChain; }
    const std::vector<VkImage>& getSwapChainImages() const { return swapChainImages; }
    VkFormat getSwapChainImageFormat() const { return swapChainImageFormat; }
    VkExtent2D getSwapChainExtent() const { return swapChainExtent; }
    const std::vector<VkImageView>& getSwapChainImageViews() const { return swapChainImageViews; }
    
    // Depth Buffer Getter 메서드들 추가
    VkImageView getDepthImageView() const { return depthImageView; }
    VkFormat getDepthFormat() const { return depthFormat; }
    
    // 유틸리티 메서드들
    uint32_t getImageCount() const { return static_cast<uint32_t>(swapChainImages.size()); }
    bool isValid() const { return swapChain != VK_NULL_HANDLE; }

    // Dynamic Rendering을 위한 헬퍼 메서드들
    VkRenderingAttachmentInfo getColorAttachmentInfo(uint32_t imageIndex) const;
    VkRenderingAttachmentInfo getDepthAttachmentInfo() const; // Depth Attachment 정보 추가
    VkRenderingInfo getRenderingInfo(uint32_t imageIndex) const;

private:
    // 내부 헬퍼 메서드들
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    
    // Depth Buffer 관련 헬퍼 메서드들 추가
    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    bool hasStencilComponent(VkFormat format);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
};