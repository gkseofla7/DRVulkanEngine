// Material.h

#pragma once // ��� ������ �ߺ����� ���ԵǴ� ���� �����մϴ�.

#include <vector>
#include <memory>
#include <vulkan/vulkan.h>

// ���� ����: ��ü ��� ������ �������� �ʰ� Ŭ������ ���縸 �˷��ݴϴ�.
// ��� ������ ������ �ӵ��� ���̰� �������� ����ϴ�.
class Texture;
class VulkanContext;
class UniformBuffer;

struct MaterialUBO {
    alignas(4) int useNormalTex;
    alignas(4) int useSpecularTex;
    alignas(4) int useAmbientTex;
    alignas(4) int useEmissiveTex;
};

class Material {
public:
    // �����ڿ� �Ҹ���
    Material(const VulkanContext* context,
        std::shared_ptr<Texture> diffuse,
        std::shared_ptr<Texture> specular,
        std::shared_ptr<Texture> normal,
        std::shared_ptr<Texture> ambient,
        std::shared_ptr<Texture> emissive);
    ~Material();

    static void initializeLayouts(VkDevice device);
    static void destroyLayouts(VkDevice device);

    static VkDescriptorSetLayout& getTextureSetLayout() { return s_textureSetLayout; }
    static VkDescriptorSetLayout& getUboSetLayout() { return s_uboSetLayout; }

    const VkDescriptorSet       getTextureSet() const { return textureSet_; }
    const VkDescriptorSet       getUboSet() const { return uboSet_; }

private:
    static void createDescriptorSetLayouts(VkDevice device );
    void createDescriptorPool();
    void createAndBindDescriptorSets();
    void createUniformBuffer();

    const VulkanContext* context_;
    std::vector<std::shared_ptr<Texture>> textures_;
    std::shared_ptr<Texture> defaultTexture_;

    static VkDescriptorSetLayout s_textureSetLayout;
    static VkDescriptorSetLayout s_uboSetLayout;

    // Set 0: Textures
    VkDescriptorSet       textureSet_;

    // Set 1: UBO
    MaterialUBO           materialUBO_;
    std::unique_ptr<UniformBuffer>       materialUB_;

    VkDescriptorSet       uboSet_;

    VkDescriptorPool      descriptorPool_;
};