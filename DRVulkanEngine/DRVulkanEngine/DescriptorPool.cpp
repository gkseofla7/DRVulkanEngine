#include "DescriptorPool.h"
#include "VulkanContext.h" // 실제 VulkanContext 정의 포함
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
    auto it = layoutCache_.find(bindings);
    if (it != layoutCache_.end()) {
        return it->second;
    }

    std::vector<VkDescriptorBindingFlags> bindingFlags(bindings.size(), 0);

    for (size_t i = 0; i < bindings.size(); ++i) {
        if (bindings[i].descriptorCount > 1) {
            bindingFlags[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
        }
    }

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{};
    flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
    flagsInfo.pBindingFlags = bindingFlags.data();


    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    layoutInfo.pNext = &flagsInfo;

    VkDescriptorSetLayout newLayout;
    if (vkCreateDescriptorSetLayout(device_, &layoutInfo, nullptr, &newLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    // 3. 새로 만든 레이아웃을 캐시에 저장하고 반환합니다.
    layoutCache_[bindings] = newLayout;
    return newLayout;
}

void DescriptorPool::initialize(VulkanContext* context)
{
    layoutCache_.initialize(context->getDevice());
    context_ = context;

    // 새로 생성할 풀의 기본 크기를 정의합니다.
    // 애플리케이션에서 주로 사용하는 디스크립터 타입과 개수를 기반으로 조절하는 것이 좋습니다.
    defaultPoolSizes_ = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 }
    };

    // 시작 시 첫 번째 풀을 생성합니다.
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
        // VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT 플래그 없이 생성된 풀만 reset 가능
        vkResetDescriptorPool(device, pool, 0);
    }
}

void DescriptorPool::createNewPool()
{
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    // 개별적으로 set을 해제할 수 있게 하는 플래그. Reset과 같이 사용하려면 주의가 필요.
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000; // 이 풀에서 할당 가능한 최대 set 개수
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

    // 1. 가장 최근에 추가된 풀부터 거꾸로 순회하며 할당을 시도합니다.
    //    (새 풀일수록 공간이 있을 확률이 높기 때문)
    for (auto it = pools_.rbegin(); it != pools_.rend(); ++it) {
        allocInfo.descriptorPool = *it;
        VkResult result = vkAllocateDescriptorSets(context_->getDevice(), &allocInfo, &descriptorSet);

        if (result == VK_SUCCESS) {
            return true; // 성공!
        }

        // 이 풀의 메모리가 부족하거나 파편화된 경우, 다른 풀에서 시도합니다.
        if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
            continue;
        }
        else {
            // 그 외의 심각한 오류인 경우
            throw std::runtime_error("failed to allocate descriptor set!");
        }
    }

    // 2. 모든 기존 풀에서 할당에 실패한 경우, 새 풀을 만듭니다.
    std::cout << "All descriptor pools are full. Creating a new one..." << std::endl;
    createNewPool();

    // 3. 방금 만든 새 풀(벡터의 맨 마지막)에서 다시 할당을 시도합니다.
    allocInfo.descriptorPool = pools_.back();
    if (vkAllocateDescriptorSets(context_->getDevice(), &allocInfo, &descriptorSet) == VK_SUCCESS) {
        return true;
    }

    // 새 풀에서도 실패했다면, 이는 메모리 부족이 아닌 다른 심각한 문제입니다.
    throw std::runtime_error("Failed to allocate from a newly created descriptor pool!");
}