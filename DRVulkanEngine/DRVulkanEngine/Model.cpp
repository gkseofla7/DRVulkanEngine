#include "Model.h"
#include "VulkanContext.h"
#include "Material.h"

Model::Model(const VulkanContext* context) {
    context_ = context; // Resource 클래스로부터 상속받은 context_
}

Model::~Model() {

}

void Model::addMesh(Mesh&& mesh) {
    meshes_.push_back(std::move(mesh));
}


void Model::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet& globalDescriptorSet){
    // 이 모델이 가진 모든 메쉬를 순회하며 그립니다.
    for (auto& mesh : meshes_) {
		mesh.draw(commandBuffer, pipelineLayout, globalDescriptorSet);
    }
}