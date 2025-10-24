#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <algorithm>

// VkDescriptorSetLayoutBinding 구조체를 해시하기 위한 함수
inline void hash_combine(size_t& seed, const VkDescriptorSetLayoutBinding& binding) {
    seed ^= std::hash<uint32_t>()(binding.binding) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<uint32_t>()(binding.descriptorType) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<uint32_t>()(binding.descriptorCount) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<uint32_t>()(binding.stageFlags) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct DescriptorLayoutInfoHasher {
    size_t operator()(const std::vector<VkDescriptorSetLayoutBinding>& bindings) const {
        // 정렬을 위해 복사본 생성
        auto sorted_bindings = bindings;
        std::sort(sorted_bindings.begin(), sorted_bindings.end(),
            [](const VkDescriptorSetLayoutBinding& a, const VkDescriptorSetLayoutBinding& b) {
                return a.binding < b.binding;
            });

        // 정렬된 벡터를 기반으로 해시 계산
        size_t seed = 0;
        for (const auto& b : sorted_bindings) {
            hash_combine(seed, b);
        }
        return seed;
    }
};

// std::vector<VkDescriptorSetLayoutBinding>를 위한 커스텀 비교 연산자
struct DescriptorLayoutInfoEqual {
    bool operator()(const std::vector<VkDescriptorSetLayoutBinding>& a, const std::vector<VkDescriptorSetLayoutBinding>& b) const {
        if (a.size() != b.size()) {
            return false;
        }

        // 정렬을 위해 복사본을 만듭니다. (원본은 const이므로 수정 불가)
        auto sorted_a = a;
        auto sorted_b = b;

        // binding 번호 기준으로 오름차순 정렬
        auto sort_func = [](const VkDescriptorSetLayoutBinding& lhs, const VkDescriptorSetLayoutBinding& rhs) {
            return lhs.binding < rhs.binding;
            };
        std::sort(sorted_a.begin(), sorted_a.end(), sort_func);
        std::sort(sorted_b.begin(), sorted_b.end(), sort_func);

        // 정렬된 벡터를 비교합니다.
        for (size_t i = 0; i < sorted_a.size(); ++i) {
            if (sorted_a[i].binding != sorted_b[i].binding ||
                sorted_a[i].descriptorType != sorted_b[i].descriptorType ||
                sorted_a[i].descriptorCount != sorted_b[i].descriptorCount ||
                sorted_a[i].stageFlags != sorted_b[i].stageFlags ||
                // pImmutableSamplers도 비교하는 것이 완벽합니다 (주로 nullptr).
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

    // 이 함수가 핵심입니다!
    VkDescriptorSetLayout getLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

private:
    VkDevice device_ = VK_NULL_HANDLE;

    std::unordered_map<
        std::vector<VkDescriptorSetLayoutBinding>,
        VkDescriptorSetLayout,
        DescriptorLayoutInfoHasher,  // 커스텀 해셔
        DescriptorLayoutInfoEqual    // 커스텀 비교 연산자
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

    // 생성된 모든 디스크립터 풀을 관리하는 벡터
    std::vector<VkDescriptorPool> pools_;

    // 새로 생성될 풀의 기본 크기 템플릿
    std::vector<VkDescriptorPoolSize> defaultPoolSizes_;
public:
	DescriptorLayoutCache layoutCache_;
};