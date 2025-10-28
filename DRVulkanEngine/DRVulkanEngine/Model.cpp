#include "Model.h"
#include "VulkanContext.h"
#include "Material.h"
#include "ModelLoader.h"
#include "UniformBuffer.h"
#include "TextureArray.h"
#include "UniformBufferArray.h"
#include "GlobalData.h"
#include "Animator.h"
#include "Animation.h"

Model::Model(const VulkanContext* context, const std::string& filedir, const std::string& filename) {
    context_ = context; // Resource 클래스로부터 상속받은 context_
    bool modelLoaded = ModelLoader::LoadSkinnedModel(context_, filedir, filename, meshes_);
    bool animLoaded = ModelLoader::LoadAnimations(context_, filedir, "mouseModelAnim.fbx", animations_);
    if (animations_.size() > 0) {
        animator_ = std::make_unique<class Animator>(&animations_[0]);
    }
    
	modelUB_ = std::make_unique<class UniformBuffer>(context_, sizeof(UniformBufferObject));
    boneUB_ = std::make_unique<class UniformBuffer>(context_, sizeof(UniformBufferBone));
}

Model::~Model() {

}


void Model::prepareBindless(UniformBufferArray& modelUbArray, UniformBufferArray& materialUbArray, UniformBufferArray& boneUbArray, TextureArray& textures)
{
    modelUbIndex_ = modelUbArray.addUniformBuffer(modelUB_.get());
	boneUbIndex_ = boneUbArray.addUniformBuffer(boneUB_.get());
    meshes_[0].prepareBindless(materialUbArray, textures);
}

void Model::addMesh(Mesh&& mesh) {
    meshes_.push_back(std::move(mesh));
}

void Model::update(float deltaTime) {
    if (animator_) {
        UniformBufferBone ubBone;
        animator_->updateAnimation(deltaTime);
        const auto& finalBoneMatrices = animator_->getFinalBoneMatrices();

        size_t matricesToCopy = std::min((size_t)MAX_BONES, finalBoneMatrices.size());

        if (!finalBoneMatrices.empty()) {
            memcpy(&ubBone.finalBoneMatrix, finalBoneMatrices.data(), sizeof(glm::mat4) * matricesToCopy);
        }
        boneUB_->update(&ubBone);
    }
}

void Model::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) {
    for (auto& mesh : meshes_) {

        PushConstantData pushData{};
        pushData.modelUBIndex = modelUbIndex_;
        pushData.boneUbIndex = boneUbIndex_;
        pushData.materialIndex = mesh.getMaterial()->getMaterialIndex();

        vkCmdPushConstants(
            commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData),
            &pushData
        );
        mesh.draw(commandBuffer, pipelineLayout);
    }
}

void Model::updateUniformBuffer(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projMatrix){
    UniformBufferObject ubo{};
    ubo.world = modelMatrix;
    ubo.view = viewMatrix;
    ubo.proj = projMatrix;
	modelUB_->update(&ubo);
}