#include "Model.h"
#include "VulkanContext.h"
#include "Material.h"
#include "ModelLoader.h"
#include "UniformBuffer.h"

Model::Model(const VulkanContext* context, const std::string& filedir, const std::string& filename) {
    context_ = context; // Resource 클래스로부터 상속받은 context_
    ModelLoader::LoadModel(context_, filedir, filename, meshes_);

	modelUB_ = std::make_unique<class UniformBuffer>(context_, sizeof(UniformBufferObject));
}



Model::~Model() {

}



void Model::prepareBindless(std::map<std::string, UniformBuffer*>& uniformBuffers_, std::map<std::string, Texture*>& textures_)
{
	uniformBuffers_["UniformBufferObject"] = modelUB_.get();
    textures_["texDiffuse"] = meshes_[0].diffuseTexture_.get();
    textures_["texSpecular"] = meshes_[0].specularTexture_.get();
    textures_["texNormal"] = meshes_[0].normalTexture_.get();
    textures_["texAmbient"] = meshes_[0].ambientTexture_.get();
    textures_["texEmissive"] = meshes_[0].emissiveTexture_.get();
}

void Model::addMesh(Mesh&& mesh) {
    meshes_.push_back(std::move(mesh));
}


void Model::draw(VkCommandBuffer commandBuffer){
    // 이 모델이 가진 모든 메쉬를 순회하며 그립니다.
    for (auto& mesh : meshes_) {
		mesh.draw(commandBuffer);
    }
}

void Model::updateUniformBuffer(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projMatrix){
    UniformBufferObject ubo{};
    ubo.world = modelMatrix;
    ubo.view = viewMatrix;
    ubo.proj = projMatrix;
	modelUB_->update(&ubo);
}