#include "Material.h"
#include "VulkanContext.h"
#include "Texture.h"
#include <array>
#include <stdexcept>
#include "UniformBuffer.h"
#include "TextureArray.h"
#include "UniformBufferArray.h"

Material::Material(const VulkanContext* context,
    std::shared_ptr<Texture> diffuse,
    std::shared_ptr<Texture> specular,
    std::shared_ptr<Texture> normal,
    std::shared_ptr<Texture> ambient,
    std::shared_ptr<Texture> emissive)
{
    context_ = context;

    // 2. 텍스처 포인터 저장
    diffuseTexture_ = diffuse;
    specularTexture_ = specular;
    normalTexture_ = normal;
    ambientTexture_ = ambient;
    emissiveTexture_ = emissive;

    createUniformBuffer();
}

Material::~Material() {
}

void Material::createUniformBuffer()
{
    static_assert(sizeof(MaterialUBO) % 16 == 0, "MaterialUBO size must be a multiple of 16 for std140 array compatibility!");
    materialUB_ = std::make_unique<UniformBuffer>(context_, sizeof(MaterialUBO));
}


void Material::prepareBindless(UniformBufferArray& uniformBufferArray, TextureArray& textures)
{
    static std::shared_ptr<Texture> defaultTexture = nullptr;
    if (defaultTexture == nullptr)
    {
        // 임시방편: 실제로는 Renderer 등 상위 클래스에서 미리 로드해야 합니다.
        defaultTexture = std::make_shared<Texture>(context_, "../assets/images/minion.jpg");
    }
    if (diffuseTexture_)
    {
        materialUBO_.diffuseTexIndex = textures.AddTexture(diffuseTexture_.get());
    }
    
    materialUBO_.diffuseTexIndex = diffuseTexture_ ? textures.AddTexture(diffuseTexture_.get()): textures.getDefualtTextureIndex();
    materialUBO_.specularTexIndex = specularTexture_ ? textures.AddTexture( specularTexture_.get()): textures.getDefualtTextureIndex();
    materialUBO_.normalTexIndex = normalTexture_ ? textures.AddTexture(normalTexture_.get()): textures.getDefualtTextureIndex();
    materialUBO_.ambientTexIndex = ambientTexture_ ? textures.AddTexture(ambientTexture_.get()): textures.getDefualtTextureIndex();
    materialUBO_.emissiveTexIndex = emissiveTexture_ ? textures.AddTexture(emissiveTexture_.get()): textures.getDefualtTextureIndex();

    materialUB_->update(&materialUBO_);
    materialIndex_ = uniformBufferArray.addUniformBuffer(materialUB_.get());
}
