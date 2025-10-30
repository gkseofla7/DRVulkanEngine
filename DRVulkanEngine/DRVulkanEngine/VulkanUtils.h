#include <vulkan/vulkan.h>
namespace VulkanUtils {

    void insertImageMemoryBarrier(
        VkCommandBuffer cmdbuffer,
        VkImage image,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkPipelineStageFlags srcStage,
        VkPipelineStageFlags dstStage,
        VkAccessFlags srcAccessMask,
        VkAccessFlags dstAccessMask);

} // namespace VulkanUtils