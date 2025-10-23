#include "Material.h"
#include "VulkanContext.h" // VkDevice�� �������� ���� �ʿ�
#include "Texture.h"       // �ؽ�ó�� imageView, sampler�� �����ϱ� ���� �ʿ�
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
    createAndBindDescriptorSets(); // �̸� ����
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

    // 1. �ؽ�ó�� ���� Descriptor Set Layout (Set 0) ����
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
            throw std::runtime_error("�ؽ�ó Descriptor Set Layout ���� ����!");
        }
    }
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0; // �� Set �ȿ����� ���ε� 0��
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &s_uboSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("UBO Descriptor Set Layout ���� ����!");
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
    poolInfo.maxSets = 2; // <<-- �߿�: 2���� Set�� �Ҵ��� ���̹Ƿ� 2�� ����

    if (vkCreateDescriptorPool(context_->getDevice(), &poolInfo, nullptr, &descriptorPool_) != VK_SUCCESS) {
        throw std::runtime_error("Descriptor Pool ���� ����!");
    }
}

void Material::createAndBindDescriptorSets() {
    // 1. �� ���� Set Layout(�ؽ�ó��, UBO��)�� �迭�� ����ϴ�.
    std::array<VkDescriptorSetLayout, 2> layouts = { s_uboSetLayout, s_textureSetLayout};

    // 2. �� ���� Set�� �� ���� �Ҵ��ϱ� ���� ������ �����մϴ�.
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool_;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size()); // �Ҵ��� Set�� ������ 2��
    allocInfo.pSetLayouts = layouts.data();

    // 3. vkAllocateDescriptorSets�� ȣ���Ͽ� Set���� �Ҵ��մϴ�.
    //    ����� �ӽ� �迭�� ���� �ް�, ��� ������ �����մϴ�.
    std::array<VkDescriptorSet, 2> allocatedSets;
    if (vkAllocateDescriptorSets(context_->getDevice(), &allocInfo, allocatedSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Descriptor Set �Ҵ� ����!");
    }
    uboSet_ = allocatedSets[0];
    textureSet_ = allocatedSets[1];
    

    // 4. �Ҵ�� Set���� ������ ������Ʈ(���ε�)�ϱ� ���� Write �������� �غ��մϴ�.
    std::vector<VkWriteDescriptorSet> descriptorWrites;

    VkDescriptorBufferInfo ubDescriptorBufferInfo = materialUB_->GetDescriptorBufferInfo();
    VkWriteDescriptorSet uboWrite{};
    uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    uboWrite.dstSet = uboSet_; // �������� 'uboSet_'
    uboWrite.dstBinding = 0;   // �� Set �ȿ����� ���ε� 0��
    uboWrite.dstArrayElement = 0;
    uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboWrite.descriptorCount = 1;
    uboWrite.pBufferInfo = &ubDescriptorBufferInfo;
    descriptorWrites.push_back(uboWrite);

    std::vector<VkDescriptorImageInfo> imageInfos(textures_.size()); // 5���� �̹��� ������ ���� ����
    for (size_t i = 0; i < textures_.size(); ++i) {
        // ����� �ؽ�ó ���� (nullptr�� ��� �⺻ �ؽ�ó ���)
        std::shared_ptr<Texture> textureToBind = textures_[i];
        if (!textureToBind) {
            // �� ������ ����Ǳ� ���� defaultTexture_�� �ݵ�� �ε�Ǿ� �־�� �մϴ�.
            if (defaultTexture_ == nullptr)
            {
                // �ӽù���: �����δ� Renderer �� ���� Ŭ�������� �̸� �ε��ؾ� �մϴ�.
                defaultTexture_ = std::make_shared<Texture>(context_, "../assets/images/minion.jpg");
            }
            textureToBind = defaultTexture_;
        }

        // �̹��� ���� ����
        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfos[i].imageView = textureToBind->getImageView();
        imageInfos[i].sampler = textureToBind->getSampler();

        // �ؽ�ó ���ε��� ���� Write ���� ����
        VkWriteDescriptorSet textureWrite{};
        textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        textureWrite.dstSet = textureSet_; // �������� 'textureSet_'
        textureWrite.dstBinding = static_cast<uint32_t>(i); // ���ε� ��ȣ�� 0, 1, 2, 3, 4
        textureWrite.dstArrayElement = 0;
        textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        textureWrite.descriptorCount = 1;
        textureWrite.pImageInfo = &imageInfos[i]; // i��° �̹��� ���� ������

        descriptorWrites.push_back(textureWrite);
    }

    // 5. �غ�� ��� Write ����(�ؽ�ó 5�� + UBO 1��)�� �� ���� ȣ��� GPU�� ������Ʈ�մϴ�.
    vkUpdateDescriptorSets(context_->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}