#include "Model.h"
#include "VulkanContext.h"
#include "Material.h"

Model::Model(const VulkanContext* context) {
    context_ = context; // Resource Ŭ�����κ��� ��ӹ��� context_
}

Model::~Model() {

}

void Model::addMesh(Mesh&& mesh) {
    meshes_.push_back(std::move(mesh));
}


void Model::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet& globalDescriptorSet){
    // �� ���� ���� ��� �޽��� ��ȸ�ϸ� �׸��ϴ�.
    for (auto& mesh : meshes_) {
		mesh.draw(commandBuffer, pipelineLayout, globalDescriptorSet);
    }
}