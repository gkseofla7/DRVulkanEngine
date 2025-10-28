#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Animation.h"

class Animator {
public:
    // 생성자: 재생할 애니메이션을 받습니다.
    Animator(Animation* animation);

    // 매 프레임 호출되어 애니메이션 시간을 업데이트하고 뼈 변환을 계산합니다.
    void updateAnimation(float dt);

    // 다른 애니메이션으로 전환합니다.
    void PlayAnimation(Animation* pAnimation);

    // 최종 계산된 뼈 변환 행렬들을 반환합니다. (이것을 셰이더로 보냅니다)
    const std::vector<glm::mat4>& getFinalBoneMatrices() const {
        return finalBoneMatrices_;
    }

private:
    // 뼈 계층 구조를 재귀적으로 순회하며 최종 변환을 계산하는 핵심 함수입니다.
    void CalculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform);

    std::vector<glm::mat4> finalBoneMatrices_; // 셰이더로 보낼 최종 행렬들
    Animation* currentAnimation_;              // 현재 재생 중인 애니메이션
    float currentTime_;                        // 현재 재생 시간 (in ticks)
};