#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "Mesh.h"
class VulkanContext;

class Model
{
public:
	friend class ModelLoader;
    // ������: Vulkan Context�� ����/�ε��� �����͸� �޽��ϴ�.
    Model(const VulkanContext* context);
    ~Model();

    void addMesh(Mesh&& mesh);

    void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet& globalDescriptorSet);
    // TODO. ���� ���� ����
private:
    const VulkanContext* context_;
    std::vector<Mesh> meshes_;
};

