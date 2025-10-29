#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Animation.h"

class Animator {
public:
    // 생성자: 재생할 애니메이션을 받습니다.
    Animator(Animation* animation);

    // 매 프레임 호출되어 애니메이션 시간을 업데이트하고 본 변환을 계산합니다.
    bool updateAnimation(float dt);

    // 다른 애니메이션으로 전환합니다.
    void PlayAnimation(Animation* pAnimation);

    // 현재 본의 본 변환 행렬들을 반환합니다. (이것을 셰이더로 보냅니다)
    const std::vector<glm::mat4>& getFinalBoneMatrices() const {
        return finalBoneMatrices_;
    }

private:
    // 본 노드 트리를 재귀적으로 순회하며 각각 변환을 적용하는 핵심 함수입니다.
    void calculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform);

    std::vector<glm::mat4> finalBoneMatrices_; // 셰이더로 보낼 본의 행렬들
    Animation* currentAnimation_;              // 현재 재생 중인 애니메이션
    float currentTime_;                        // 현재 재생 시간 (in ticks)
    float lastTime_;                          // 이전 프레임의 시간 (변경 감지용)
    
    // 성능 최적화를 위한 캐싱
    static constexpr float TIME_EPSILON = 0.001f; // 시간 변경 감지 임계값
    bool forceUpdate_;                            // 강제 업데이트 플래그
};