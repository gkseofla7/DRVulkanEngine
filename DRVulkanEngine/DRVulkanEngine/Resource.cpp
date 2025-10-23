#include "Resource.h"
#include "VulkanContext.h"
#include <stdexcept>

void Resource::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    // 1. 버퍼 생성 정보(CreateInfo) 정의
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size; // 버퍼의 크기 (바이트 단위)
    bufferInfo.usage = usage; // 버퍼의 용도 (예: 정점 버퍼, 복사 원본 등)
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // 단일 큐에서만 사용 (간단한 설정)

    // 2. 버퍼 핸들 생성
    if (vkCreateBuffer(context->getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    // 3. 버퍼에 필요한 메모리 요구사항 쿼리
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context->getDevice(), buffer, &memRequirements);

    // 4. 메모리 할당 정보(AllocateInfo) 정의
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size; // 필요한 메모리 크기
    // 요구사항과 속성을 만족하는 적절한 메모리 타입을 찾습니다.
    allocInfo.memoryTypeIndex = context->findMemoryType(memRequirements.memoryTypeBits, properties);

    // 5. GPU 메모리 할당
    if (vkAllocateMemory(context->getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    // 6. 할당된 메모리를 버퍼에 바인딩(연결)
    vkBindBufferMemory(context->getDevice(), buffer, bufferMemory, 0);
}

VkCommandBuffer Resource::beginSingleTimeCommands() {
    // 1. 커맨드 풀에서 커맨드 버퍼를 하나 할당받습니다.
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = context->getCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(context->getDevice(), &allocInfo, &commandBuffer);

    // 2. 커맨드 버퍼 기록을 시작합니다.
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // 이 버퍼는 한 번만 사용하고 파괴될 것임

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Resource::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    // 1. 커맨드 버퍼 기록을 종료합니다.
    vkEndCommandBuffer(commandBuffer);

    // 2. 제출 정보(Submit Info)를 정의합니다.
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // 3. 그래픽스 큐에 커맨드 버퍼를 제출합니다.
    vkQueueSubmit(context->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    // 4. 작업이 완료될 때까지 CPU가 기다리도록 동기화합니다.
    vkQueueWaitIdle(context->getGraphicsQueue());

    // 5. 사용이 끝난 임시 커맨드 버퍼를 해제합니다.
    vkFreeCommandBuffers(context->getDevice(), context->getCommandPool(), 1, &commandBuffer);
}
