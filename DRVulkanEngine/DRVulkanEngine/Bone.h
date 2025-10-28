#pragma once

#include <vector>
#include <string>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// 위치 키프레임
struct KeyPosition {
    glm::vec3 position;
    float timeStamp;
};

// 회전 키프레임
struct KeyRotation {
    glm::quat orientation;
    float timeStamp;
};

// 스케일 키프레임
struct KeyScale {
    glm::vec3 scale;
    float timeStamp;
};

struct BoneInfo {
    // 뼈의 고유 ID (셰이더에서 행렬 배열의 인덱스로 사용됨)
    int id;

    // 오프셋 행렬 (Inverse Bind Pose Matrix)
    // 모델 공간의 정점을 해당 뼈의 로컬 공간으로 변환하는 행렬입니다.
    glm::mat4 offsetMatrix;
};

class Bone {
public:
    // 생성자: Assimp의 aiNodeAnim 데이터로부터 키프레임들을 읽어옵니다.
    Bone(const std::string& name, int ID, const aiNodeAnim* channel);

    // Update: 특정 애니메이션 시간(in ticks)에 맞춰 보간된 로컬 변환 행렬을 계산합니다.
    void Update(float animationTime);

    // Getters
    glm::mat4 GetLocalTransform() const { return m_LocalTransform; }
    std::string GetBoneName() const { return m_Name; }
    int GetBoneID() const { return m_ID; }

private:
    // 키프레임 간의 보간(Interpolation)을 위한 헬퍼 함수들
    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
    glm::mat4 InterpolatePosition(float animationTime);
    glm::mat4 InterpolateRotation(float animationTime);
    glm::mat4 InterpolateScaling(float animationTime);

    // 뼈의 모든 키프레임 데이터
    std::vector<KeyPosition> m_Positions;
    std::vector<KeyRotation> m_Rotations;
    std::vector<KeyScale> m_Scales;

    // 현재 시간에 맞춰 계산된 최종 로컬 변환 행렬
    glm::mat4 m_LocalTransform;
    std::string m_Name;
    int m_ID;
};