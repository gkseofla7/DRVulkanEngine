#include "Animator.h"
#include "ModelLoader.h"

Animator::Animator(Animation* animation) {
    currentTime_ = 0.0;
    lastTime_ = -1.0f; // 초기값을 -1로 설정하여 첫 번째 업데이트를 보장
    forceUpdate_ = true; // 초기화 시 강제 업데이트
    currentAnimation_ = animation;

    finalBoneMatrices_.resize(100, glm::mat4(1.0f));
}

void Animator::PlayAnimation(Animation* pAnimation) {
    if (currentAnimation_ != pAnimation) { // 같은 애니메이션이면 변경하지 않음
        currentAnimation_ = pAnimation;
        currentTime_ = 0.0f;
        lastTime_ = -1.0f; // 애니메이션 변경 시 강제 업데이트를 위해 리셋
        forceUpdate_ = true;
    }
}

bool Animator::updateAnimation(float dt) {
    if (!currentAnimation_) {
        return false;
    }

    // 1. 시간 업데이트
    float previousTime = currentTime_;
    
    // 시간 기반 업데이트로 변경 (더 정확한 애니메이션)
    currentTime_ += currentAnimation_->GetTicksPerSecond() * dt;
    currentTime_ = fmod(currentTime_, currentAnimation_->GetDuration());
    
    // 시간 변경 체크 - 임계값 기반 비교로 부동소수점 오차 처리
    bool timeChanged = forceUpdate_ || 
                      (lastTime_ < 0.0f) || 
                      (std::abs(currentTime_ - lastTime_) >= TIME_EPSILON);
    
    if (!timeChanged) {
        return false; // 시간이 유의미하게 변경되지 않았으면 업데이트 생략
    }
    
    lastTime_ = currentTime_;
    forceUpdate_ = false;
    
    // 2. 실제 본의 변환 행렬 등을 계산
    calculateBoneTransform(&currentAnimation_->GetRootNode(), ModelLoader::globalInverseTransform_);
    
    return true; // 애니메이션이 업데이트됨
}

void Animator::calculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform) {
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

    // 자식 노드들 처리 - 스택 깊이를 고려한 재귀 호출
    for (const auto& child : node->children) {
        calculateBoneTransform(&child, globalTransformation);
    }
}