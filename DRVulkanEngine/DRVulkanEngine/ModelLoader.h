#pragma once

#include <vector>
#include <string>
#include <map>
#include "Mesh.h"
#include "Animation.h"
#include "Bone.h" // BoneInfo 구조체가 여기에 정의되어 있다고 가정

// 전방 선언
class VulkanContext;
struct aiNode;
struct aiScene;
struct aiMesh;

class ModelLoader {
public:
    // 1. 스켈레톤과 메시 데이터를 모델 파일에서 로드합니다.
    static bool LoadSkinnedModel(const VulkanContext* context, const std::string& filedir, const std::string& filename, std::vector<Mesh>& outMesh);

    // 2. 애니메이션 데이터를 별도의 파일에서 로드합니다.
    static bool LoadAnimations(const VulkanContext* context, const std::string& filedir, const std::string& filename, std::vector<Animation>& outAnimations);

private:
    // 기존 헬퍼 함수들을 static으로 변경
    static void processNode(const VulkanContext* context, aiNode* node, const aiScene* scene, const std::string& filedir, std::vector<Mesh>& outMesh);
    static void processMesh(const VulkanContext* context, aiMesh* mesh, const aiScene* scene, const std::string& filedir, std::vector<Mesh>& outMeshes);
    static void setVertexBoneData(Vertex& vertex, int boneID, float weight);
    static void extractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
    static void processAnimations(const aiScene* scene, std::vector<Animation>& outAnimations);

    // 이 static 멤버들이 두 함수 호출 사이의 데이터를 연결하는 다리 역할을 합니다.
    static std::map<std::string, BoneInfo> boneInfoMap_;
    static int boneCounter_;
public:
	static glm::mat4 globalInverseTransform_;
};