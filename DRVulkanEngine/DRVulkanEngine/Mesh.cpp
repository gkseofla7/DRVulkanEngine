#include "Mesh.h"
#include "VulkanContext.h"
#include "TextureArray.h"
#include <stdexcept>

Mesh::Mesh() {
}

Mesh::~Mesh() {
    vkDestroyBuffer(context_->getDevice(), indexBuffer_, nullptr);
    vkFreeMemory(context_->getDevice(), indexBufferMemory_, nullptr);
    vkDestroyBuffer(context_->getDevice(), vertexBuffer_, nullptr);
    vkFreeMemory(context_->getDevice(), vertexBufferMemory_, nullptr);
}

Mesh::Mesh(Mesh&& other) noexcept
    : vertexBuffer_(other.vertexBuffer_),
    vertexBufferMemory_(other.vertexBufferMemory_),
    indexBuffer_(other.indexBuffer_),
    indexBufferMemory_(other.indexBufferMemory_),
    vertices_(std::move(other.vertices_)),
    indices_(std::move(other.indices_)),
    context_(other.context_),
    material_(std::move(other.material_))
{
    other.vertexBuffer_ = VK_NULL_HANDLE;
    other.vertexBufferMemory_ = VK_NULL_HANDLE;
    other.indexBuffer_ = VK_NULL_HANDLE;
    other.indexBufferMemory_ = VK_NULL_HANDLE;
}

void Mesh::update(float dt)
{

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

    createVertexBuffer();
    createIndexBuffer();

    if(inDiffuse.get() != nullptr)
    {
        material_ = std::make_unique<Material>(context,
            inDiffuse,
            inSpecular,
            inNormal,
            inAmbient,
            inEmissive);
	}
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
    VkDeviceSize bufferSize = sizeof(indices_[0]) * indices_.size();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(context_->getDevice(), &bufferInfo, nullptr, &indexBuffer_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create index buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context_->getDevice(), indexBuffer_, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = context_->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(context_->getDevice(), &allocInfo, nullptr, &indexBufferMemory_) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate index buffer memory!");
    }

    vkBindBufferMemory(context_->getDevice(), indexBuffer_, indexBufferMemory_, 0);

    // 이제 데이터를 버퍼에 복사합니다.
    void* data;
    vkMapMemory(context_->getDevice(), indexBufferMemory_, 0, bufferSize, 0, &data);
    // 3. 원본 데이터를 'vertices' 대신 'indices' 벡터에서 가져옵니다.
    memcpy(data, indices_.data(), (size_t)bufferSize);
    vkUnmapMemory(context_->getDevice(), indexBufferMemory_);
}

void Mesh::prepareBindless(UniformBufferArray& uniformBufferArray, TextureArray& textures)
{
	material_->prepareBindless(uniformBufferArray, textures);
}
