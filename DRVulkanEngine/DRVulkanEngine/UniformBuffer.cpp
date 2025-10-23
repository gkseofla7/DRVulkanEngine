#include "UniformBuffer.h"
#include "VulkanContext.h" // context->getDevice() ���� ����ϱ� ���� ����
#include <stdexcept>
#include <cstring> // memcpy�� ����ϱ� ���� ����

UniformBuffer::UniformBuffer(const VulkanContext* ctx, VkDeviceSize size) {
    // 1. �θ� Ŭ������ ��� ������ context�� �����մϴ�.
    context = ctx;
    bufferSize_ = size;

    // 2. �θ� Ŭ������ createBuffer ���� �Լ��� ȣ���Ͽ� UBO�� �����մϴ�.
    // UBO�� GPU���� �а� CPU���� ���� ������Ʈ�ϹǷ�,
    // HOST_VISIBLE�� HOST_COHERENT �Ӽ��� ���� �޸𸮸� ����մϴ�.
    createBuffer(bufferSize_,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        buffer_,
        bufferMemory_);
}

UniformBuffer::~UniformBuffer() {
    // 3. �Ҹ� ��, ������ ���ۿ� �޸𸮸� �ݵ�� �����Ͽ� �޸� ������ �����մϴ�.
    if (buffer_ != VK_NULL_HANDLE) {
        vkDestroyBuffer(context->getDevice(), buffer_, nullptr);
    }
    if (bufferMemory_ != VK_NULL_HANDLE) {
        vkFreeMemory(context->getDevice(), bufferMemory_, nullptr);
    }
}

void UniformBuffer::update(const void* data) {
    // 4. GPU �޸𸮸� CPU���� ���� ������ �����ͷ� �����մϴ�.
    void* mappedMemory;
    vkMapMemory(context->getDevice(), bufferMemory_, 0, bufferSize_, 0, &mappedMemory);

    // 5. CPU�� �����͸� ���ε� GPU �޸� �����Ϳ� �����մϴ�.
    memcpy(mappedMemory, data, static_cast<size_t>(bufferSize_));

    // 6. �޸� ������ �����մϴ�.
    // (HOST_COHERENT �Ӽ� ���п� vkFlushMappedMemoryRanges�� ȣ���� �ʿ䰡 �����ϴ�.)
    vkUnmapMemory(context->getDevice(), bufferMemory_);
}