#include "Mesh.h"
#include "VulkanContext.h"
#include <stdexcept>

Mesh::Mesh() {}

Mesh::~Mesh() {
    vkDestroyBuffer(context_->getDevice(), indexBuffer_, nullptr);
    vkFreeMemory(context_->getDevice(), indexBufferMemory_, nullptr);
    vkDestroyBuffer(context_->getDevice(), vertexBuffer_, nullptr);
    vkFreeMemory(context_->getDevice(), vertexBufferMemory_, nullptr);
}
Mesh::Mesh(Mesh&& other) noexcept
// 1. �ٸ� ��ü(other)�� ��� ����� ���� ��ü(this)�� �̵�(������ ����)��ŵ�ϴ�.
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
    material_(std::move(other.material_)) // unique_ptr�� �������� ����
{
    other.vertexBuffer_ = VK_NULL_HANDLE;
    other.vertexBufferMemory_ = VK_NULL_HANDLE;
    other.indexBuffer_ = VK_NULL_HANDLE;
    other.indexBufferMemory_ = VK_NULL_HANDLE;
}

void Mesh::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet& globalDescriptorSet)
{
    VkBuffer vertexBuffers[] = { vertexBuffer_ };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer_, 0, VK_INDEX_TYPE_UINT32);

    std::array<VkDescriptorSet, 3> descriptorSetsToBind = {
        globalDescriptorSet,
        material_->getUboSet(),
        material_->getTextureSet()
    };
    vkCmdBindDescriptorSets(commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        0, // firstSet: ��ũ���� �� ���̾ƿ� �迭�� ���� �ε���
        3, // descriptorSetCount: ���ε��� ��ũ���� ���� ����
        descriptorSetsToBind.data(),
        0, // dynamicOffsetCount
        nullptr); // pDynamicOffsets

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
    // 1. �⺻ ���� �� ������ ����
    context_ = context;
    vertices_ = inVertices;
    indices_ = inIndices;

    // 2. �ؽ�ó ������ ����
    diffuseTexture_ = inDiffuse;
    specularTexture_ = inSpecular;
    normalTexture_ = inNormal;
    ambientTexture_ = inAmbient;
    emissiveTexture_ = inEmissive;

    // 3. Vertex �� Index ���� ����
    createVertexBuffer();
    createIndexBuffer();

    // 4. ����� �ؽ�ó���� ����Ͽ� Material(Descriptor Set) ����
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
    // 1. ������ ũ�⸦ 'indices' ���͸� �������� ����մϴ�.
    VkDeviceSize bufferSize = sizeof(indices_[0]) * indices_.size();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    // 2. ������ �뵵�� VK_BUFFER_USAGE_INDEX_BUFFER_BIT�� �����մϴ�.
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // 'indexBuffer_' �ڵ鿡 ���۸� �����մϴ�.
    if (vkCreateBuffer(context_->getDevice(), &bufferInfo, nullptr, &indexBuffer_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create index buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context_->getDevice(), indexBuffer_, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = context_->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // 'indexBufferMemory_' �ڵ鿡 �޸𸮸� �Ҵ��մϴ�.
    if (vkAllocateMemory(context_->getDevice(), &allocInfo, nullptr, &indexBufferMemory_) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate index buffer memory!");
    }

    // ������ ���ۿ� �޸𸮸� ����(���ε�)�մϴ�.
    vkBindBufferMemory(context_->getDevice(), indexBuffer_, indexBufferMemory_, 0);

    // ���� �����͸� ���ۿ� �����մϴ�.
    void* data;
    vkMapMemory(context_->getDevice(), indexBufferMemory_, 0, bufferSize, 0, &data);
    // 3. ���� �����͸� 'vertices' ��� 'indices' ���Ϳ��� �����ɴϴ�.
    memcpy(data, indices_.data(), (size_t)bufferSize);
    vkUnmapMemory(context_->getDevice(), indexBufferMemory_);
}