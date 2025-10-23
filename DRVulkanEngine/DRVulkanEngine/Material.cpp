#include "Material.h"
#include "VulkanContext.h" // VkDevice를 가져오기 위해 필요
#include "Texture.h"       // 텍스처의 imageView, sampler에 접근하기 위해 필요
#include <array>
#include <stdexcept>
#include "UniformBuffer.h"

VkDescriptorSetLayout Material::s_textureSetLayout = VK_NULL_HANDLE;
VkDescriptorSetLayout Material::s_uboSetLayout = VK_NULL_HANDLE;

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
    createDescriptorPool();
    createAndBindDescriptorSets(); // 이름 변경
}

Material::~Material() {
    VkDevice device = context_->getDevice();
    vkDestroyDescriptorPool(device, descriptorPool_, nullptr);
}

void Material::createUniformBuffer()
{
    materialUB_ = std::make_unique<UniformBuffer>(context_, sizeof(MaterialUBO));
    materialUB_->update(&materialUBO_);
}

void Material::initializeLayouts(VkDevice device)
{
    createDescriptorSetLayouts(device);
}
void Material::destroyLayouts(VkDevice device)
{
    vkDestroyDescriptorSetLayout(device, s_uboSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, s_textureSetLayout, nullptr);
    s_uboSetLayout = VK_NULL_HANDLE;
    s_textureSetLayout = VK_NULL_HANDLE;
}

void Material::createDescriptorSetLayouts(VkDevice device) {

    // 1. 텍스처를 위한 Descriptor Set Layout (Set 0) 생성
    {
        std::array<VkDescriptorSetLayoutBinding, 5> bindings = {};
        for (size_t i = 0; i < bindings.size(); ++i) {
            bindings[i].binding = static_cast<uint32_t>(i);
            bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindings[i].descriptorCount = 1;
            bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &s_textureSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("텍스처 Descriptor Set Layout 생성 실패!");
        }
    }
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0; // 이 Set 안에서는 바인딩 0번
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &s_uboSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("UBO Descriptor Set Layout 생성 실패!");
        }
    }
}

void Material::createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 5;


    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 2; // <<-- 중요: 2개의 Set을 할당할 것이므로 2로 설정

    if (vkCreateDescriptorPool(context_->getDevice(), &poolInfo, nullptr, &descriptorPool_) != VK_SUCCESS) {
        throw std::runtime_error("Descriptor Pool 생성 실패!");
    }
}

void Material::createAndBindDescriptorSets() {
    // 1. 두 개의 Set Layout(텍스처용, UBO용)을 배열에 담습니다.
    std::array<VkDescriptorSetLayout, 2> layouts = { s_uboSetLayout, s_textureSetLayout};

    // 2. 두 개의 Set을 한 번에 할당하기 위한 정보를 설정합니다.
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool_;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size()); // 할당할 Set의 개수는 2개
    allocInfo.pSetLayouts = layouts.data();

    // 3. vkAllocateDescriptorSets를 호출하여 Set들을 할당합니다.
    //    결과는 임시 배열에 먼저 받고, 멤버 변수에 저장합니다.
    std::array<VkDescriptorSet, 2> allocatedSets;
    if (vkAllocateDescriptorSets(context_->getDevice(), &allocInfo, allocatedSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Descriptor Set 할당 실패!");
    }
    uboSet_ = allocatedSets[0];
    textureSet_ = allocatedSets[1];
    

    // 4. 할당된 Set들의 내용을 업데이트(바인딩)하기 위한 Write 정보들을 준비합니다.
    std::vector<VkWriteDescriptorSet> descriptorWrites;

    VkDescriptorBufferInfo ubDescriptorBufferInfo = materialUB_->GetDescriptorBufferInfo();
    VkWriteDescriptorSet uboWrite{};
    uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    uboWrite.dstSet = uboSet_; // 목적지는 'uboSet_'
    uboWrite.dstBinding = 0;   // 이 Set 안에서는 바인딩 0번
    uboWrite.dstArrayElement = 0;
    uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboWrite.descriptorCount = 1;
    uboWrite.pBufferInfo = &ubDescriptorBufferInfo;
    descriptorWrites.push_back(uboWrite);

    std::vector<VkDescriptorImageInfo> imageInfos(textures_.size()); // 5개의 이미지 정보를 담을 벡터
    for (size_t i = 0; i < textures_.size(); ++i) {
        // 사용할 텍스처 결정 (nullptr일 경우 기본 텍스처 사용)
        std::shared_ptr<Texture> textureToBind = textures_[i];
        if (!textureToBind) {
            // 이 로직이 실행되기 전에 defaultTexture_가 반드시 로드되어 있어야 합니다.
            if (defaultTexture_ == nullptr)
            {
                // 임시방편: 실제로는 Renderer 등 상위 클래스에서 미리 로드해야 합니다.
                defaultTexture_ = std::make_shared<Texture>(context_, "../assets/images/minion.jpg");
            }
            textureToBind = defaultTexture_;
        }

        // 이미지 정보 설정
        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfos[i].imageView = textureToBind->getImageView();
        imageInfos[i].sampler = textureToBind->getSampler();

        // 텍스처 바인딩을 위한 Write 정보 생성
        VkWriteDescriptorSet textureWrite{};
        textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        textureWrite.dstSet = textureSet_; // 목적지는 'textureSet_'
        textureWrite.dstBinding = static_cast<uint32_t>(i); // 바인딩 번호는 0, 1, 2, 3, 4
        textureWrite.dstArrayElement = 0;
        textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        textureWrite.descriptorCount = 1;
        textureWrite.pImageInfo = &imageInfos[i]; // i번째 이미지 정보 포인터

        descriptorWrites.push_back(textureWrite);
    }

    // 5. 준비된 모든 Write 정보(텍스처 5개 + UBO 1개)를 한 번의 호출로 GPU에 업데이트합니다.
    vkUpdateDescriptorSets(context_->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}