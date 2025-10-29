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
#include "PrimitiveFactory.h"
#include <glm/gtc/type_ptr.hpp> // value_ptr을 위한 헤더 추가

Model::Model(const VulkanContext* context, const ModelConfig& modelConfig) {
    context_ = context; // Resource 클래스로부터 상속받은 context_
    if(modelConfig.type == ModelType::FromFile) {
        std::string filedir = modelConfig.modelDirectory;
        std::string filename = modelConfig.modelFilename;
        
        bool modelLoaded = ModelLoader::LoadSkinnedModel(context_, filedir, filename, meshes_);
        for (const auto& animFilename : modelConfig.animationFilenames) {
            ModelLoader::LoadAnimations(context_, filedir, animFilename, animations_);
            if (animations_.size() > 0) {
                animator_ = std::make_unique<class Animator>(&animations_[0]);
            }
        }
    }
    else if(modelConfig.type == ModelType::Box) {
        
        PrimitiveFactory::createBox(context, 50, 50, 50, meshes_);
	}

	modelUB_ = std::make_unique<class UniformBuffer>(context_, sizeof(UniformBufferObject));
    boneUB_ = std::make_unique<class UniformBuffer>(context_, sizeof(UniformBufferBone));
    
    // 성능 최적화를 위한 초기화
    framesSinceLastBoneUpdate_ = 0;
    boneDataDirty_ = true;
    cachedBoneMatrices_.reserve(MAX_BONES); // 메모리 재할당 방지
    
    memset(&ubBoneBuffer_, 0, sizeof(UniformBufferBone));
}

Model::~Model() {

}

// 빠른 매트릭스 비교를 위한 인라인 함수
inline bool Model::isMatrixChanged(const glm::mat4& a, const glm::mat4& b) const {
    // 더 효율적인 메모리 비교 방식
    return memcmp(glm::value_ptr(a), glm::value_ptr(b), sizeof(glm::mat4)) != 0;
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
    if (!animator_) {
        return;
    }

    ++framesSinceLastBoneUpdate_;
    if (framesSinceLastBoneUpdate_ < BONE_UPDATE_FREQUENCY) {
        return;
    }
    framesSinceLastBoneUpdate_ = 0;

    bool animationChanged = animator_->updateAnimation(deltaTime);
    
    if (!animationChanged && !boneDataDirty_) {
        return;
    }

    const auto& finalBoneMatrices = animator_->getFinalBoneMatrices();
    
    if (finalBoneMatrices.empty()) {
        return;
    }

    const size_t matricesToProcess = std::min((size_t)MAX_BONES, finalBoneMatrices.size());
    
    if (!boneDataDirty_ && cachedBoneMatrices_.size() == finalBoneMatrices.size()) {
        bool hasChanges = false;
        
        for (size_t i = 0; i < matricesToProcess && !hasChanges; ++i) {
            if (isMatrixChanged(cachedBoneMatrices_[i], finalBoneMatrices[i])) {
                hasChanges = true;
            }
        }
        
        if (!hasChanges) {
            return; // 변경사항이 없으면 GPU 업데이트 생략
        }
    }

    if (cachedBoneMatrices_.size() != finalBoneMatrices.size()) {
        cachedBoneMatrices_.resize(finalBoneMatrices.size());
    }
    
    if (matricesToProcess > 0) {
        memcpy(cachedBoneMatrices_.data(), 
               finalBoneMatrices.data(), 
               sizeof(glm::mat4) * matricesToProcess);
        
        memcpy(&ubBoneBuffer_.finalBoneMatrix, 
               finalBoneMatrices.data(), 
               sizeof(glm::mat4) * matricesToProcess);
        
        boneUB_->update(&ubBoneBuffer_);
    }

    boneDataDirty_ = false;
}

void Model::draw(VkCommandBuffer commandBuffer) {
    for (auto& mesh : meshes_) {
        mesh.draw(commandBuffer);
    }
}

void Model::getPushConstantData(PushConstantData& outPushData)
{
	outPushData.modelUBIndex = modelUbIndex_;
	outPushData.boneUbIndex = boneUbIndex_;
	outPushData.materialIndex = meshes_[0].getMaterial() ? meshes_[0].getMaterial()->getMaterialIndex() : -1;
}

void Model::updateUniformBuffer(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projMatrix){
    UniformBufferObject ubo{};
    ubo.world = modelMatrix;
    ubo.view = viewMatrix;
    ubo.proj = projMatrix;
	modelUB_->update(&ubo);
}