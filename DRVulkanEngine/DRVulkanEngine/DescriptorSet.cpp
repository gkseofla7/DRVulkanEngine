#include "DescriptorSet.h"
#include "Resource.h"
#include "VulkanContext.h"
#include "DescriptorPool.h"
#include <stdexcept>
#include <vector>

void DescriptorSet::initialize(VulkanContext* inContext, DescriptorPool* inDescriptorPool, VkDescriptorSetLayout inDescriptorSetLayout, const std::vector<Resource*>& inResources)
{
    context_ = inContext;

    // 1. 관리되는 풀(Pool)로부터 디스크립터 셋을 할당받습니다.
    //    VulkanContext가 DescriptorPoolManager를 가지고 있다고 가정합니다.
    bool allocated = inDescriptorPool->allocateDescriptorSet(inDescriptorSetLayout, descriptorSet_);
    if (!allocated) {
        throw std::runtime_error("Failed to allocate descriptor set from pool!");
    }

    // 2. 할당받은 디스크립터 셋에 실제 리소스들을 연결(업데이트)합니다.
    updateSet(inResources);
}

// User의 createDescriptorSets() 요청을 이 함수로 구현
void DescriptorSet::updateSet(const std::vector<Resource*>& resources)
{
    // VkWriteDescriptorSet: "어떤 디스크립터 셋의 몇 번 바인딩에 어떤 리소스를 연결할지"에 대한 정보
    std::vector<VkWriteDescriptorSet> descriptorWrites;
    descriptorWrites.reserve(resources.size());

    int index = 0;
    for (const auto& resource : resources) {
        VkWriteDescriptorSet writeInfo{};
		resource->populateWriteDescriptor(writeInfo);
        writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeInfo.dstSet = descriptorSet_;
        writeInfo.dstBinding = index++;
        writeInfo.dstArrayElement = 0;
        descriptorWrites.push_back(writeInfo);
    }

    // 준비된 정보들을 바탕으로 디스크립터 셋 업데이트를 한번에 실행
    vkUpdateDescriptorSets(context_->getDevice(),
        static_cast<uint32_t>(descriptorWrites.size()),
        descriptorWrites.data(),
        0, nullptr);
}