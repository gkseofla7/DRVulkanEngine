#define GLM_ENABLE_EXPERIMENTAL
#include "Animation.h"
#include "ModelLoader.h" // BoneInfo 구조체를 사용하기 위해 포함
#include <glm/gtx/string_cast.hpp> // glm::to_string을 위해 필요
#include <iostream>
// 생성자
Animation::Animation(const aiScene* scene, aiAnimation* animation, 
                     const std::map<std::string, BoneInfo>& boneInfoMap)
{
    m_Duration = animation->mDuration;
    m_TicksPerSecond = animation->mTicksPerSecond;
    ReadHierarchyData(m_RootNode, scene->mRootNode);
    
    // 전달받은 올바른 BoneInfo 맵을 사용
    m_BoneInfoMap = boneInfoMap;
    
    for (unsigned int i = 0; i < animation->mNumChannels; i++) {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;
        
        // 애니메이션에만 존재하는 뼈 처리
        if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end()) {
            BoneInfo newBoneInfo;
            newBoneInfo.id = m_BoneInfoMap.size();
            newBoneInfo.offsetMatrix = glm::mat4(1.0f);
            m_BoneInfoMap[boneName] = newBoneInfo;
        }
        
        m_Bones.emplace_back(boneName, m_BoneInfoMap[boneName].id, channel);
    }
}

// 이름으로 Bone 객체를 찾습니다.
Bone* Animation::FindBone(const std::string& name) {
    for (auto& bone : m_Bones) {
        if (bone.GetBoneName() == name) {
            return &bone;
        }
    }
    return nullptr;
}

// 재귀적으로 노드를 순회하며 뼈의 부모-자식 관계를 저장합니다.
void Animation::ReadHierarchyData(AssimpNodeData& dest, const aiNode* src) {
    dest.name = src->mName.data;

    const auto& mat = src->mTransformation;
    dest.transformation = glm::mat4(
        mat.a1, mat.b1, mat.c1, mat.d1,
        mat.a2, mat.b2, mat.c2, mat.d2,
        mat.a3, mat.b3, mat.c3, mat.d3,
        mat.a4, mat.b4, mat.c4, mat.d4);
    for (unsigned int i = 0; i < src->mNumChildren; i++) {
        AssimpNodeData newData;
        ReadHierarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}