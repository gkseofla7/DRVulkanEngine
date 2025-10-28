#pragma once

#include <vector>
#include <map>
#include <string>
#include <assimp/scene.h>
#include "Bone.h" 

// BoneInfo 구조체는 ModelLoader.h나 별도의 공용 헤더에 있어야 합니다.
// #include "ModelLoader.h" 또는 #include "AnimationData.h" 가 필요할 수 있습니다.
struct BoneInfo;

// Assimp의 노드 계층 구조를 저장하기 위한 구조체
struct AssimpNodeData {
    glm::mat4 transformation;
    std::string name;
    std::vector<AssimpNodeData> children;
};

class Animation {
public:
    Animation() = default;

    // 생성자를 수정하여 BoneInfo 맵을 직접 받도록 함
    Animation(const aiScene* scene, aiAnimation* animation, 
              const std::map<std::string, BoneInfo>& boneInfoMap);

    ~Animation() = default;

    // 이름으로 Bone 객체(연기 대본)를 찾습니다.
    Bone* FindBone(const std::string& name);

    // Getters
    float GetTicksPerSecond() const { return m_TicksPerSecond; }
    float GetDuration() const { return m_Duration; }
    const AssimpNodeData& GetRootNode() const { return m_RootNode; }
    const std::map<std::string, BoneInfo>& GetBoneIDMap() const { return m_BoneInfoMap; }

private:
    // aiNode 계층 구조를 재귀적으로 읽어와 m_RootNode에 저장합니다.
    void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);

    float m_Duration;
    float m_TicksPerSecond;
    std::vector<Bone> m_Bones; // 이 애니메이션에 포함된 모든 Bone 객체들
    AssimpNodeData m_RootNode;
    std::map<std::string, BoneInfo> m_BoneInfoMap; // 모델 전체의 BoneInfo 맵 복사본
};