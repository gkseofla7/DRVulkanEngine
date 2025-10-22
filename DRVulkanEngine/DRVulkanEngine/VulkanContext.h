#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <string>

// GLFW 전방 선언
struct GLFWwindow;

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class VulkanContext {
private:
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    
    // Debug 관련 멤버 추가
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    
    // Debug 모드 설정
    #ifdef _DEBUG
        const bool enableValidationLayers = true;
    #else
        const bool enableValidationLayers = false;
    #endif
    
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

public:
    VulkanContext() = default;
    ~VulkanContext();

    // 복사 생성자와 대입 연산자 삭제 (RAII 패턴)
    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

    // 이동 생성자와 이동 대입 연산자
    VulkanContext(VulkanContext&& other) noexcept;
    VulkanContext& operator=(VulkanContext&& other) noexcept;

    // 초기화 및 정리
    void createInstance();
    void createSurface(GLFWwindow* window);
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void cleanup();

    // Getter 메서드들
    VkInstance getInstance() const { return instance; }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    VkDevice getDevice() const { return device; }
    VkQueue getGraphicsQueue() const { return graphicsQueue; }
    VkQueue getPresentQueue() const { return presentQueue; }
    VkSurfaceKHR getSurface() const { return surface; }
    VkCommandPool getCommandPool() const { return commandPool; }

    // 유틸리티 메서드들
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
    const uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

private:
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    
    // Debug 관련 Private 메서드들
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();
    
    // Debug 콜백 함수
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
};