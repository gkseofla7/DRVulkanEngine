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
    // ���� �ε� �Լ� (Public API)
    static std::unique_ptr<Model> LoadModel(const VulkanContext* context, const std::string& filedir, const std::string& filepath);

private:
    // �� �׷����� ��������� ��ȸ�ϴ� �Լ�
    static void processNode(const VulkanContext* context, aiNode* node, const aiScene* scene, const std::string& filedir, Model& outModel);

    // ���� �޽��� DREngine�� Mesh ��ü�� ��ȯ�ϴ� �Լ�
    static Mesh processMesh(const VulkanContext* context, aiMesh* mesh, const aiScene* scene, const std::string& filedir);
};