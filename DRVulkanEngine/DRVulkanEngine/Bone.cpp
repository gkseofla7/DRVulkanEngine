#define GLM_ENABLE_EXPERIMENTAL
#include "Bone.h"
#include <glm/gtx/quaternion.hpp>

// 생성자: Assimp 데이터로부터 키프레임을 복사합니다.
Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel)
    : m_Name(name), m_ID(ID), m_LocalTransform(1.0f)
{
    // 위치 키프레임 복사
    for (unsigned int i = 0; i < channel->mNumPositionKeys; ++i) {
        aiVector3D pos = channel->mPositionKeys[i].mValue;
        float timeStamp = channel->mPositionKeys[i].mTime;
        m_Positions.push_back({ glm::vec3(pos.x, pos.y, pos.z), (float)timeStamp });
    }

    // 회전 키프레임 복사
    for (unsigned int i = 0; i < channel->mNumRotationKeys; ++i) {
        aiQuaternion orient = channel->mRotationKeys[i].mValue;
        float timeStamp = channel->mRotationKeys[i].mTime;
        m_Rotations.push_back({ glm::quat(orient.w, orient.x, orient.y, orient.z), (float)timeStamp });
    }

    // 스케일 키프레임 복사
    for (unsigned int i = 0; i < channel->mNumScalingKeys; ++i) {
        aiVector3D scale = channel->mScalingKeys[i].mValue;
        float timeStamp = channel->mScalingKeys[i].mTime;
        m_Scales.push_back({ glm::vec3(scale.x, scale.y, scale.z), (float)timeStamp });
    }
}

// 매 프레임 호출되어 현재 시간에 맞는 변환 행렬을 계산합니다.
void Bone::Update(float animationTime) {
    glm::mat4 translation = InterpolatePosition(animationTime);
    glm::mat4 rotation = InterpolateRotation(animationTime);
    glm::mat4 scale = InterpolateScaling(animationTime);
    m_LocalTransform = translation*rotation* scale;
}

// 위치 보간 (선형 보간 - Lerp)
glm::mat4 Bone::InterpolatePosition(float animationTime) {
    if (m_Positions.size() == 1)
        return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

    int p0Index = -1;
    for (unsigned int i = 0; i < m_Positions.size() - 1; ++i) {
        if (animationTime < m_Positions[i + 1].timeStamp) {
            p0Index = i;
            break;
        }
    }
    // 애니메이션 시간이 마지막 키프레임 시간을 넘었을 경우 마지막 위치를 사용
    if (p0Index == -1) {
        return glm::translate(glm::mat4(1.0f), m_Positions.back().position);
    }
    int p1Index = p0Index + 1;

    float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp, m_Positions[p1Index].timeStamp, animationTime);
    glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position, m_Positions[p1Index].position, scaleFactor);

    return glm::translate(glm::mat4(1.0f), finalPosition);
    //return glm::translate(glm::mat4(1.0f), m_Positions[p0Index].position);
}

// 회전 보간 (구면 선형 보간 - Slerp)
glm::mat4 Bone::InterpolateRotation(float animationTime) {
    if (m_Rotations.size() == 1)
        return glm::toMat4(glm::normalize(m_Rotations[0].orientation));

    int r0Index = -1;
    for (unsigned int i = 0; i < m_Rotations.size() - 1; ++i) {
        if (animationTime < m_Rotations[i + 1].timeStamp) {
            r0Index = i;
            break;
        }
    }
    // 애니메이션 시간이 마지막 키프레임 시간을 넘었을 경우 마지막 회전을 사용
    if (r0Index == -1) {
        return glm::toMat4(glm::normalize(m_Rotations.back().orientation));
    }
    int r1Index = r0Index + 1;

    float scaleFactor = GetScaleFactor(m_Rotations[r0Index].timeStamp, m_Rotations[r1Index].timeStamp, animationTime);
    glm::quat finalRotation = glm::slerp(m_Rotations[r0Index].orientation, m_Rotations[r1Index].orientation, scaleFactor);
    finalRotation = glm::normalize(finalRotation);
    return glm::toMat4(finalRotation);
    //return glm::toMat4(m_Rotations[r0Index].orientation);
}

// 스케일 보간 (선형 보간 - Lerp)
glm::mat4 Bone::InterpolateScaling(float animationTime) {
    if (m_Scales.size() == 1)
        return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

    int s0Index = -1;
    for (unsigned int i = 0; i < m_Scales.size() - 1; ++i) {
        if (animationTime < m_Scales[i + 1].timeStamp) {
            s0Index = i;
            break;
        }
    }
    // 애니메이션 시간이 마지막 키프레임 시간을 넘었을 경우 마지막 스케일을 사용
    if (s0Index == -1) {
        return glm::scale(glm::mat4(1.0f), m_Scales.back().scale);
    }
    int s1Index = s0Index + 1;

    float scaleFactor = GetScaleFactor(m_Scales[s0Index].timeStamp, m_Scales[s1Index].timeStamp, animationTime);
    glm::vec3 finalScale = glm::mix(m_Scales[s0Index].scale, m_Scales[s1Index].scale, scaleFactor);

    return glm::scale(glm::mat4(1.0f), finalScale);
    //return glm::scale(glm::mat4(1.0f), m_Scales[s0Index].scale);
}

float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) {
    float framesDiff = nextTimeStamp - lastTimeStamp;
    if (framesDiff == 0.0f) {
        return 0.0f;
    }
    float midWayLength = animationTime - lastTimeStamp;
    return midWayLength / framesDiff;
}