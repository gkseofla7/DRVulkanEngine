#include "DescriptorPool.h"
#include "VulkanContext.h" // ���� VulkanContext ���� ����
#include <iostream>

void DescriptorLayoutCache::initialize(VkDevice device) {
    device_ = device;
}

void DescriptorLayoutCache::destroy() {
    for (auto const& [key, layout] : layoutCache_) {
        vkDestroyDescriptorSetLayout(device_, layout, nullptr);
    }
    layoutCache_.clear();
}

VkDescriptorSetLayout DescriptorLayoutCache::getLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings) {
    // 1. ĳ�ÿ��� ���� ã�ƺ��ϴ�.
    auto it = layoutCache_.find(bindings);
    if (it != layoutCache_.end()) {
        return it->second; // ã������ �ٷ� ��ȯ
    }

    // 2. ĳ�ÿ� ���ٸ� ���� �����մϴ�.
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    VkDescriptorSetLayout newLayout;
    if (vkCreateDescriptorSetLayout(device_, &layoutInfo, nullptr, &newLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    // 3. ���� ���� ���̾ƿ��� ĳ�ÿ� �����ϰ� ��ȯ�մϴ�.
    layoutCache_[bindings] = newLayout;
    return newLayout;
}

void DescriptorPool::initialize(VulkanContext* context)
{
    layoutCache_.initialize(context->getDevice());
    context_ = context;

    // ���� ������ Ǯ�� �⺻ ũ�⸦ �����մϴ�.
    // ���ø����̼ǿ��� �ַ� ����ϴ� ��ũ���� Ÿ�԰� ������ ������� �����ϴ� ���� �����ϴ�.
    defaultPoolSizes_ = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 }
    };

    // ���� �� ù ��° Ǯ�� �����մϴ�.
    createNewPool();
}

void DescriptorPool::destroy()
{
    VkDevice device = context_->getDevice();
    for (VkDescriptorPool pool : pools_) {
        vkDestroyDescriptorPool(device, pool, nullptr);
    }
    pools_.clear();
}

void DescriptorPool::resetPools()
{
    VkDevice device = context_->getDevice();
    for (VkDescriptorPool pool : pools_) {
        // VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT �÷��� ���� ������ Ǯ�� reset ����
        vkResetDescriptorPool(device, pool, 0);
    }
}

void DescriptorPool::createNewPool()
{
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    // ���������� set�� ������ �� �ְ� �ϴ� �÷���. Reset�� ���� ����Ϸ��� ���ǰ� �ʿ�.
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000; // �� Ǯ���� �Ҵ� ������ �ִ� set ����
    poolInfo.poolSizeCount = static_cast<uint32_t>(defaultPoolSizes_.size());
    poolInfo.pPoolSizes = defaultPoolSizes_.data();

    VkDescriptorPool newPool;
    if (vkCreateDescriptorPool(context_->getDevice(), &poolInfo, nullptr, &newPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    std::cout << "Descriptor pool created. Total pools: " << pools_.size() + 1 << std::endl;
    pools_.push_back(newPool);
}

bool DescriptorPool::allocateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet& descriptorSet)
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    // 1. ���� �ֱٿ� �߰��� Ǯ���� �Ųٷ� ��ȸ�ϸ� �Ҵ��� �õ��մϴ�.
    //    (�� Ǯ�ϼ��� ������ ���� Ȯ���� ���� ����)
    for (auto it = pools_.rbegin(); it != pools_.rend(); ++it) {
        allocInfo.descriptorPool = *it;
        VkResult result = vkAllocateDescriptorSets(context_->getDevice(), &allocInfo, &descriptorSet);

        if (result == VK_SUCCESS) {
            return true; // ����!
        }

        // �� Ǯ�� �޸𸮰� �����ϰų� ����ȭ�� ���, �ٸ� Ǯ���� �õ��մϴ�.
        if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
            continue;
        }
        else {
            // �� ���� �ɰ��� ������ ���
            throw std::runtime_error("failed to allocate descriptor set!");
        }
    }

    // 2. ��� ���� Ǯ���� �Ҵ翡 ������ ���, �� Ǯ�� ����ϴ�.
    std::cout << "All descriptor pools are full. Creating a new one..." << std::endl;
    createNewPool();

    // 3. ��� ���� �� Ǯ(������ �� ������)���� �ٽ� �Ҵ��� �õ��մϴ�.
    allocInfo.descriptorPool = pools_.back();
    if (vkAllocateDescriptorSets(context_->getDevice(), &allocInfo, &descriptorSet) == VK_SUCCESS) {
        return true;
    }

    // �� Ǯ������ �����ߴٸ�, �̴� �޸� ������ �ƴ� �ٸ� �ɰ��� �����Դϴ�.
    throw std::runtime_error("Failed to allocate from a newly created descriptor pool!");
}