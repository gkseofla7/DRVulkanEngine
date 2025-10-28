#include "Animator.h"
#include "ModelLoader.h"

Animator::Animator(Animation* animation) {
    currentTime_ = 0.0;
    currentAnimation_ = animation;

    finalBoneMatrices_.resize(100, glm::mat4(1.0f));
}

void Animator::PlayAnimation(Animation* pAnimation) {
    currentAnimation_ = pAnimation;
    currentTime_ = 0.0f;
}

void Animator::updateAnimation(float dt) {
    if (currentAnimation_) {
        // 1. 시간 업데이트
        //currentTime_ += currentAnimation_->GetTicksPerSecond() * dt;
        currentTime_++;
        currentTime_ = fmod(currentTime_, currentAnimation_->GetDuration());
        // 2. 뼈의 최종 변환 행렬 계산 시작
        CalculateBoneTransform(&currentAnimation_->GetRootNode(), ModelLoader::globalInverseTransform_);
    }
}

void Animator::CalculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform) {
    std::string nodeName = node->name;
    Bone* bone = currentAnimation_->FindBone(nodeName);

    glm::mat4 nodeTransform;
    if (bone) {
        bone->Update(currentTime_);
        nodeTransform = bone->GetLocalTransform();
    }
    else {
        // ReadHierarchyData에서 이미 전치된 행렬을 그대로 사용
        nodeTransform = node->transformation;
    }

    // parentTransform (시작은 globalInverse)에 자식 변환을 계속 곱해나감
    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    auto boneInfoMap = currentAnimation_->GetBoneIDMap();
    if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
        int boneID = boneInfoMap[nodeName].id;
        const glm::mat4& offsetMatrix = boneInfoMap[nodeName].offsetMatrix;
        finalBoneMatrices_[boneID] = globalTransformation * offsetMatrix;
    }

    for (const auto& child : node->children) {
        CalculateBoneTransform(&child, globalTransformation);
    }
}