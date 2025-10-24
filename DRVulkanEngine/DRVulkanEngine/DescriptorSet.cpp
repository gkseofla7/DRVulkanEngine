#include "DescriptorSet.h"
#include "Resource.h"
#include "VulkanContext.h"
#include "DescriptorPool.h"
#include <stdexcept>
#include <vector>

void DescriptorSet::initialize(VulkanContext* inContext, DescriptorPool* inDescriptorPool, VkDescriptorSetLayout inDescriptorSetLayout, const std::vector<Resource*>& inResources)
{
    context_ = inContext;

    // 1. �����Ǵ� Ǯ(Pool)�κ��� ��ũ���� ���� �Ҵ�޽��ϴ�.
    //    VulkanContext�� DescriptorPoolManager�� ������ �ִٰ� �����մϴ�.
    bool allocated = inDescriptorPool->allocateDescriptorSet(inDescriptorSetLayout, descriptorSet_);
    if (!allocated) {
        throw std::runtime_error("Failed to allocate descriptor set from pool!");
    }

    // 2. �Ҵ���� ��ũ���� �¿� ���� ���ҽ����� ����(������Ʈ)�մϴ�.
    updateSet(inResources);
}

// User�� createDescriptorSets() ��û�� �� �Լ��� ����
void DescriptorSet::updateSet(const std::vector<Resource*>& resources)
{
    // VkWriteDescriptorSet: "� ��ũ���� ���� �� �� ���ε��� � ���ҽ��� ��������"�� ���� ����
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
        writeInfo.descriptorCount = 1;
        descriptorWrites.push_back(writeInfo);
    }

    // �غ�� �������� �������� ��ũ���� �� ������Ʈ�� �ѹ��� ����
    vkUpdateDescriptorSets(context_->getDevice(),
        static_cast<uint32_t>(descriptorWrites.size()),
        descriptorWrites.data(),
        0, nullptr);
}