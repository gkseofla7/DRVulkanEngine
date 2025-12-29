#include "UniformBuffer.h"
#include "VulkanContext.h"
#include <stdexcept>
#include <cstring>

UniformBuffer::UniformBuffer(const VulkanContext* ctx, VkDeviceSize size) {
    // 1. 부모 클래스의 멤버 변수인 context를 설정합니다.
    context = ctx;
    bufferSize_ = size;

    // 2. 부모 클래스의 createBuffer 헬퍼 함수를 호출하여 UBO를 생성합니다.
    // UBO는 GPU에서 읽고 CPU에서 자주 업데이트하므로,
    // HOST_VISIBLE과 HOST_COHERENT 속성을 가진 메모리를 사용합니다.
    createBuffer(bufferSize_,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        buffer_,
        bufferMemory_);

    bufferInfo_.buffer = buffer_;
    bufferInfo_.offset = 0;
    bufferInfo_.range = bufferSize_;
}

UniformBuffer::~UniformBuffer() {
    // 3. 소멸 시, 생성된 버퍼와 메모리를 반드시 해제하여 메모리 누수를 방지합니다.
    if (buffer_ != VK_NULL_HANDLE) {
        vkDestroyBuffer(context->getDevice(), buffer_, nullptr);
    }
    if (bufferMemory_ != VK_NULL_HANDLE) {
        vkFreeMemory(context->getDevice(), bufferMemory_, nullptr);
    }
}

void UniformBuffer::update(const void* data) {
    // 4. GPU 메모리를 CPU에서 접근 가능한 포인터로 매핑합니다.
    void* mappedMemory;
    vkMapMemory(context->getDevice(), bufferMemory_, 0, bufferSize_, 0, &mappedMemory);

    // 5. CPU의 데이터를 매핑된 GPU 메모리 포인터에 복사합니다.
    memcpy(mappedMemory, data, static_cast<size_t>(bufferSize_));

    // 6. 메모리 매핑을 해제합니다.
    // (HOST_COHERENT 속성 덕분에 vkFlushMappedMemoryRanges를 호출할 필요가 없습니다.)
    vkUnmapMemory(context->getDevice(), bufferMemory_);
}

void UniformBuffer::populateWriteDescriptor(VkWriteDescriptorSet& writeInfo) const
{
    writeInfo.descriptorCount = 1;
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.dstBinding = binding_;
    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeInfo.pBufferInfo = &bufferInfo_;
    writeInfo.pImageInfo = nullptr;
}