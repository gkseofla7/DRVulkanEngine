#pragma once
#include <vulkan/vulkan.h>
#include "Vertex.h"
#include "Material.h"
#include <vector>
#include <memory>
#include <map>
class VulkanContext;
class Texture;
class TextureArray;
class UniformBufferArray;

class Mesh
{
public:
	friend class ModelLoader;
    Mesh();
    ~Mesh();
    Mesh(Mesh&& other) noexcept;
    void update(float dt);
    void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);

	Material* getMaterial() const { return material_.get(); }
    void prepareBindless(UniformBufferArray& uniformBufferArray, TextureArray& textures);
private:
    void initialize(const VulkanContext* context,
        const std::vector<Vertex>& inVertices,
        const std::vector<uint32_t>& inIndices,
        std::shared_ptr<Texture> inDiffuse,
        std::shared_ptr<Texture> inSpecular,
        std::shared_ptr<Texture> inNormal,
        std::shared_ptr<Texture> inAmbient,
        std::shared_ptr<Texture> inEmissive);
    void intializeMaterial();
    void createVertexBuffer();
    void createIndexBuffer();
private:
    VkBuffer vertexBuffer_;
    VkDeviceMemory vertexBufferMemory_;
    VkBuffer indexBuffer_;
    VkDeviceMemory indexBufferMemory_;

    std::vector<Vertex> vertices_;
    std::vector<uint32_t> indices_;

    const VulkanContext* context_;
public:
    std::unique_ptr<Material> material_;
};

