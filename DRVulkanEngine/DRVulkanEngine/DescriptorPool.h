#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <algorithm>

// VkDescriptorSetLayoutBinding ����ü�� �ؽ��ϱ� ���� �Լ�
inline void hash_combine(size_t& seed, const VkDescriptorSetLayoutBinding& binding) {
    seed ^= std::hash<uint32_t>()(binding.binding) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<uint32_t>()(binding.descriptorType) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<uint32_t>()(binding.descriptorCount) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<uint32_t>()(binding.stageFlags) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct DescriptorLayoutInfoHasher {
    size_t operator()(const std::vector<VkDescriptorSetLayoutBinding>& bindings) const {
        // ������ ���� ���纻 ����
        auto sorted_bindings = bindings;
        std::sort(sorted_bindings.begin(), sorted_bindings.end(),
            [](const VkDescriptorSetLayoutBinding& a, const VkDescriptorSetLayoutBinding& b) {
                return a.binding < b.binding;
            });

        // ���ĵ� ���͸� ������� �ؽ� ���
        size_t seed = 0;
        for (const auto& b : sorted_bindings) {
            hash_combine(seed, b);
        }
        return seed;
    }
};

// std::vector<VkDescriptorSetLayoutBinding>�� ���� Ŀ���� �� ������
struct DescriptorLayoutInfoEqual {
    bool operator()(const std::vector<VkDescriptorSetLayoutBinding>& a, const std::vector<VkDescriptorSetLayoutBinding>& b) const {
        if (a.size() != b.size()) {
            return false;
        }

        // ������ ���� ���纻�� ����ϴ�. (������ const�̹Ƿ� ���� �Ұ�)
        auto sorted_a = a;
        auto sorted_b = b;

        // binding ��ȣ �������� �������� ����
        auto sort_func = [](const VkDescriptorSetLayoutBinding& lhs, const VkDescriptorSetLayoutBinding& rhs) {
            return lhs.binding < rhs.binding;
            };
        std::sort(sorted_a.begin(), sorted_a.end(), sort_func);
        std::sort(sorted_b.begin(), sorted_b.end(), sort_func);

        // ���ĵ� ���͸� ���մϴ�.
        for (size_t i = 0; i < sorted_a.size(); ++i) {
            if (sorted_a[i].binding != sorted_b[i].binding ||
                sorted_a[i].descriptorType != sorted_b[i].descriptorType ||
                sorted_a[i].descriptorCount != sorted_b[i].descriptorCount ||
                sorted_a[i].stageFlags != sorted_b[i].stageFlags ||
                // pImmutableSamplers�� ���ϴ� ���� �Ϻ��մϴ� (�ַ� nullptr).
                sorted_a[i].pImmutableSamplers != sorted_b[i].pImmutableSamplers) {
                return false;
            }
        }

        return true;
    }
};

class DescriptorLayoutCache {
public:
    void initialize(VkDevice device);
    void destroy();

    // �� �Լ��� �ٽ��Դϴ�!
    VkDescriptorSetLayout getLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

private:
    VkDevice device_ = VK_NULL_HANDLE;

    std::unordered_map<
        std::vector<VkDescriptorSetLayoutBinding>,
        VkDescriptorSetLayout,
        DescriptorLayoutInfoHasher,  // Ŀ���� �ؼ�
        DescriptorLayoutInfoEqual    // Ŀ���� �� ������
    > layoutCache_;
};

class VulkanContext;

class DescriptorPool
{
public:
    void initialize(VulkanContext* context);
    void destroy();

    bool allocateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet& descriptorSet);

    void resetPools();

private:
    void createNewPool();

private:
    VulkanContext* context_ = nullptr;

    // ������ ��� ��ũ���� Ǯ�� �����ϴ� ����
    std::vector<VkDescriptorPool> pools_;

    // ���� ������ Ǯ�� �⺻ ũ�� ���ø�
    std::vector<VkDescriptorPoolSize> defaultPoolSizes_;
public:
	DescriptorLayoutCache layoutCache_;
};