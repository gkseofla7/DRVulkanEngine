// Material.h

#pragma once // 헤더 파일이 중복으로 포함되는 것을 방지합니다.

#include <vector>
#include <memory>
#include <vulkan/vulkan.h>

// 전방 선언: 전체 헤더 파일을 포함하지 않고 클래스의 존재만 알려줍니다.
// 헤더 파일의 컴파일 속도를 높이고 의존성을 낮춥니다.
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
    // 생성자와 소멸자
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