#pragma once 

#include <vector>
#include <memory>
#include <vulkan/vulkan.h>
#include <map>
#include <string>
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

    // TODO. ���� ���� ����, �ӽ� �׽�Ʈ��
    void prepareBindless(std::map<std::string, UniformBuffer*>& uniformBuffers_, std::map<std::string, Texture*>& textures_);
private:
    void createUniformBuffer();

    const VulkanContext* context_;
    std::vector<std::shared_ptr<Texture>> textures_;
    std::shared_ptr<Texture> defaultTexture_;

    // Set 1: UBO
    MaterialUBO           materialUBO_;
    std::unique_ptr<UniformBuffer>       materialUB_;
};