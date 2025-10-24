#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class VulkanContext;
struct Resource;
struct DescriptorPool;

class DescriptorSet
{
public:
    // 디스크립터 셋을 초기화합니다. (풀에서 할당 & 리소스 업데이트)
    void initialize(VulkanContext* inContext, DescriptorPool* inDescriptorPool, VkDescriptorSetLayout inDescriptorSetLayout, const std::vector<Resource*>& inResources);

    // Vulkan 핸들을 가져오는 Getter
    VkDescriptorSet getHandle() const { return descriptorSet_; }

private:
    // 할당된 디스크립터 셋에 실제 리소스 정보를 기록(업데이트)합니다.
    void updateSet(const std::vector<Resource*>& resources);

private:
    VulkanContext* context_ = nullptr;
    VkDescriptorSet descriptorSet_ = VK_NULL_HANDLE;
};