#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class VulkanContext;
struct Resource;
struct DescriptorPool;

class DescriptorSet
{
public:
    // ��ũ���� ���� �ʱ�ȭ�մϴ�. (Ǯ���� �Ҵ� & ���ҽ� ������Ʈ)
    void initialize(VulkanContext* inContext, DescriptorPool* inDescriptorPool, VkDescriptorSetLayout inDescriptorSetLayout, const std::vector<Resource*>& inResources);

    // Vulkan �ڵ��� �������� Getter
    VkDescriptorSet getHandle() const { return descriptorSet_; }

private:
    // �Ҵ�� ��ũ���� �¿� ���� ���ҽ� ������ ���(������Ʈ)�մϴ�.
    void updateSet(const std::vector<Resource*>& resources);

private:
    VulkanContext* context_ = nullptr;
    VkDescriptorSet descriptorSet_ = VK_NULL_HANDLE;
};