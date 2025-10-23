#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <vector>

#include "Vertex.h" 
#include "Model.h" 
struct aiNode;
struct aiMesh;
struct aiScene;
class ModelLoader {
public:
    // 메인 로딩 함수 (Public API)
    static std::unique_ptr<Model> LoadModel(const VulkanContext* context, const std::string& filedir, const std::string& filepath);

private:
    // 씬 그래프를 재귀적으로 순회하는 함수
    static void processNode(const VulkanContext* context, aiNode* node, const aiScene* scene, const std::string& filedir, Model& outModel);

    // 단일 메쉬를 DREngine의 Mesh 객체로 변환하는 함수
    static Mesh processMesh(const VulkanContext* context, aiMesh* mesh, const aiScene* scene, const std::string& filedir);
};