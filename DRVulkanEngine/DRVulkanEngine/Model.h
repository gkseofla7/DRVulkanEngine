#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "Mesh.h"
class VulkanContext;

class Model
{
public:
	friend class ModelLoader;
    // 생성자: Vulkan Context와 정점/인덱스 데이터를 받습니다.
    Model(const VulkanContext* context);
    ~Model();

    void addMesh(Mesh&& mesh);

    void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet& globalDescriptorSet);
    // TODO. 추후 삭제 예정
private:
    const VulkanContext* context_;
    std::vector<Mesh> meshes_;
};

