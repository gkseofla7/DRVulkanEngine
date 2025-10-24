#include "Material.h"
#include "VulkanContext.h" // VkDevice�� �������� ���� �ʿ�
#include "Texture.h"       // �ؽ�ó�� imageView, sampler�� �����ϱ� ���� �ʿ�
#include <array>
#include <stdexcept>
#include "UniformBuffer.h"


Material::Material(const VulkanContext* context,
    std::shared_ptr<Texture> diffuse,
    std::shared_ptr<Texture> specular,
    std::shared_ptr<Texture> normal,
    std::shared_ptr<Texture> ambient,
    std::shared_ptr<Texture> emissive)
{
    context_ = context;
    textures_ = { diffuse, specular, normal, ambient, emissive };
    materialUBO_.useNormalTex = (normal != nullptr);
    materialUBO_.useSpecularTex = (specular != nullptr);
    materialUBO_.useAmbientTex = (ambient != nullptr);
    materialUBO_.useEmissiveTex = (emissive != nullptr);

    createUniformBuffer();
}

Material::~Material() {
}

void Material::createUniformBuffer()
{
    materialUB_ = std::make_unique<UniformBuffer>(context_, sizeof(MaterialUBO));
    materialUB_->update(&materialUBO_);
}


void Material::prepareBindless(std::map<std::string, UniformBuffer*>& uniformBuffers_, std::map<std::string, Texture*>& textures_)
{
	uniformBuffers_["MaterialUBO"] = materialUB_.get(); 
}
