#pragma once 

#include <vector>
#include <memory>
#include <vulkan/vulkan.h>
#include <map>
#include <string>
class Texture;
class VulkanContext;
class UniformBuffer;
class TextureArray;
class UniformBufferArray;

struct MaterialUBO {
    alignas(16) int diffuseTexIndex = -1;
    int normalTexIndex = -1;
    int specularTexIndex = -1;
    int ambientTexIndex = -1;
    int emissiveTexIndex = -1;
};

class Material {
public:
    // 持失切人 社瑚切
    Material(const VulkanContext* context,
        std::shared_ptr<Texture> diffuse,
        std::shared_ptr<Texture> specular,
        std::shared_ptr<Texture> normal,
        std::shared_ptr<Texture> ambient,
        std::shared_ptr<Texture> emissive);
    ~Material();

    void prepareBindless(UniformBufferArray& uniformBufferArray, TextureArray& textures);

	int getMaterialIndex() const { return materialIndex_; }
private:
    void createUniformBuffer();

    const VulkanContext* context_;
    std::shared_ptr<Texture> diffuseTexture_;
    std::shared_ptr<Texture> specularTexture_;
    std::shared_ptr<Texture> normalTexture_;
    std::shared_ptr<Texture> ambientTexture_;
    std::shared_ptr<Texture> emissiveTexture_;
    std::shared_ptr<Texture> defaultTexture_;

    MaterialUBO           materialUBO_;
    std::unique_ptr<UniformBuffer>       materialUB_;

    int materialIndex_ = 0;
};