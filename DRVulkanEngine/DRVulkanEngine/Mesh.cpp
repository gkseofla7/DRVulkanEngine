#include "Mesh.h"
#include "VulkanContext.h"
#include "Texture.h"
#include <stdexcept>

Mesh::Mesh() {}

Mesh::~Mesh() {
    vkDestroyBuffer(context_->getDevice(), indexBuffer_, nullptr);
    vkFreeMemory(context_->getDevice(), indexBufferMemory_, nullptr);
    vkDestroyBuffer(context_->getDevice(), vertexBuffer_, nullptr);
    vkFreeMemory(context_->getDevice(), vertexBufferMemory_, nullptr);
}
Mesh::Mesh(Mesh&& other) noexcept
// 1. 다른 객체(other)의 모든 멤버를 현재 객체(this)로 이동(소유권 이전)시킵니다.
    : vertexBuffer_(other.vertexBuffer_),
    vertexBufferMemory_(other.vertexBufferMemory_),
    indexBuffer_(other.indexBuffer_),
    indexBufferMemory_(other.indexBufferMemory_),
    vertices_(std::move(other.vertices_)),
    indices_(std::move(other.indices_)),
    context_(other.context_),
    diffuseTexture_(std::move(other.diffuseTexture_)),
    specularTexture_(std::move(other.specularTexture_)),
    normalTexture_(std::move(other.normalTexture_)),
    ambientTexture_(std::move(other.ambientTexture_)),
    emissiveTexture_(std::move(other.emissiveTexture_)),
    material_(std::move(other.material_)) // unique_ptr의 소유권을 이전
{
    other.vertexBuffer_ = VK_NULL_HANDLE;
    other.vertexBufferMemory_ = VK_NULL_HANDLE;
    other.indexBuffer_ = VK_NULL_HANDLE;
    other.indexBufferMemory_ = VK_NULL_HANDLE;
}

void Mesh::draw(VkCommandBuffer commandBuffer)
{
    VkBuffer vertexBuffers[] = { vertexBuffer_ };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer_, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices_.size()), 1, 0, 0, 0);
}

void Mesh::initialize(const VulkanContext* context,
    const std::vector<Vertex>& inVertices,
    const std::vector<uint32_t>& inIndices,
    std::shared_ptr<Texture> inDiffuse,
    std::shared_ptr<Texture> inSpecular,
    std::shared_ptr<Texture> inNormal,
    std::shared_ptr<Texture> inAmbient,
    std::shared_ptr<Texture> inEmissive)
{
    // 1. 기본 정보 및 데이터 저장
    context_ = context;
    vertices_ = inVertices;
    indices_ = inIndices;

    // 2. 텍스처 포인터 저장
    diffuseTexture_ = inDiffuse;
    specularTexture_ = inSpecular;
    normalTexture_ = inNormal;
    ambientTexture_ = inAmbient;
    emissiveTexture_ = inEmissive;

    // 3. Vertex 및 Index 버퍼 생성
    createVertexBuffer();
    createIndexBuffer();

    // 4. 저장된 텍스처들을 사용하여 Material(Descriptor Set) 생성
    material_ = std::make_unique<Material>(context,
        diffuseTexture_,
        specularTexture_,
        normalTexture_,
        ambientTexture_,
        emissiveTexture_);
}
void Mesh::intializeMaterial()
{

}

void Mesh::createVertexBuffer() {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(vertices_[0]) * vertices_.size();
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(context_->getDevice(), &bufferInfo, nullptr, &vertexBuffer_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context_->getDevice(), vertexBuffer_, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = context_->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(context_->getDevice(), &allocInfo, nullptr, &vertexBufferMemory_) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(context_->getDevice(), vertexBuffer_, vertexBufferMemory_, 0);

    void* data;
    vkMapMemory(context_->getDevice(), vertexBufferMemory_, 0, bufferInfo.size, 0, &data);
    memcpy(data, vertices_.data(), (size_t)bufferInfo.size);
    vkUnmapMemory(context_->getDevice(), vertexBufferMemory_);
}


void Mesh::createIndexBuffer() {
    // 1. 버퍼의 크기를 'indices' 벡터를 기준으로 계산합니다.
    VkDeviceSize bufferSize = sizeof(indices_[0]) * indices_.size();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    // 2. 버퍼의 용도를 VK_BUFFER_USAGE_INDEX_BUFFER_BIT로 설정합니다.
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // 'indexBuffer_' 핸들에 버퍼를 생성합니다.
    if (vkCreateBuffer(context_->getDevice(), &bufferInfo, nullptr, &indexBuffer_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create index buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context_->getDevice(), indexBuffer_, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = context_->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // 'indexBufferMemory_' 핸들에 메모리를 할당합니다.
    if (vkAllocateMemory(context_->getDevice(), &allocInfo, nullptr, &indexBufferMemory_) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate index buffer memory!");
    }

    // 생성된 버퍼와 메모리를 연결(바인딩)합니다.
    vkBindBufferMemory(context_->getDevice(), indexBuffer_, indexBufferMemory_, 0);

    // 이제 데이터를 버퍼에 복사합니다.
    void* data;
    vkMapMemory(context_->getDevice(), indexBufferMemory_, 0, bufferSize, 0, &data);
    // 3. 원본 데이터를 'vertices' 대신 'indices' 벡터에서 가져옵니다.
    memcpy(data, indices_.data(), (size_t)bufferSize);
    vkUnmapMemory(context_->getDevice(), indexBufferMemory_);
}

void Mesh::prepareBindless(std::map<std::string, UniformBuffer*>& uniformBuffers_, std::map<std::string, Texture*>& textures_)
{
	static std::shared_ptr<Texture> defaultTexture = nullptr;
    if (defaultTexture) {
        if (defaultTexture == nullptr)
        {
            // 임시방편: 실제로는 Renderer 등 상위 클래스에서 미리 로드해야 합니다.
            defaultTexture = std::make_shared<Texture>(context_, "../assets/images/minion.jpg");
        }
    }
    textures_["texDiffuse"] = defaultTexture.get();
    textures_["texSpecular"] = defaultTexture.get();
    textures_["texNormal"] = defaultTexture.get();
    textures_["texAmbient"] = defaultTexture.get();
    textures_["texEmissive"] = defaultTexture.get();

    textures_["texDiffuse"] = diffuseTexture_.get();
    textures_["texSpecular"] = specularTexture_.get();
    textures_["texNormal"] = normalTexture_.get();
    textures_["texAmbient"] = ambientTexture_.get();
    textures_["texEmissive"] = emissiveTexture_.get();

	material_->prepareBindless(uniformBuffers_, textures_);
}
