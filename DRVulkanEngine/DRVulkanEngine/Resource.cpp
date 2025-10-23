#include "Resource.h"
#include "VulkanContext.h"
#include <stdexcept>

void Resource::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    // 1. ���� ���� ����(CreateInfo) ����
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size; // ������ ũ�� (����Ʈ ����)
    bufferInfo.usage = usage; // ������ �뵵 (��: ���� ����, ���� ���� ��)
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // ���� ť������ ��� (������ ����)

    // 2. ���� �ڵ� ����
    if (vkCreateBuffer(context->getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    // 3. ���ۿ� �ʿ��� �޸� �䱸���� ����
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context->getDevice(), buffer, &memRequirements);

    // 4. �޸� �Ҵ� ����(AllocateInfo) ����
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size; // �ʿ��� �޸� ũ��
    // �䱸���װ� �Ӽ��� �����ϴ� ������ �޸� Ÿ���� ã���ϴ�.
    allocInfo.memoryTypeIndex = context->findMemoryType(memRequirements.memoryTypeBits, properties);

    // 5. GPU �޸� �Ҵ�
    if (vkAllocateMemory(context->getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    // 6. �Ҵ�� �޸𸮸� ���ۿ� ���ε�(����)
    vkBindBufferMemory(context->getDevice(), buffer, bufferMemory, 0);
}

VkCommandBuffer Resource::beginSingleTimeCommands() {
    // 1. Ŀ�ǵ� Ǯ���� Ŀ�ǵ� ���۸� �ϳ� �Ҵ�޽��ϴ�.
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = context->getCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(context->getDevice(), &allocInfo, &commandBuffer);

    // 2. Ŀ�ǵ� ���� ����� �����մϴ�.
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // �� ���۴� �� ���� ����ϰ� �ı��� ����

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Resource::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    // 1. Ŀ�ǵ� ���� ����� �����մϴ�.
    vkEndCommandBuffer(commandBuffer);

    // 2. ���� ����(Submit Info)�� �����մϴ�.
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // 3. �׷��Ƚ� ť�� Ŀ�ǵ� ���۸� �����մϴ�.
    vkQueueSubmit(context->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    // 4. �۾��� �Ϸ�� ������ CPU�� ��ٸ����� ����ȭ�մϴ�.
    vkQueueWaitIdle(context->getGraphicsQueue());

    // 5. ����� ���� �ӽ� Ŀ�ǵ� ���۸� �����մϴ�.
    vkFreeCommandBuffers(context->getDevice(), context->getCommandPool(), 1, &commandBuffer);
}
