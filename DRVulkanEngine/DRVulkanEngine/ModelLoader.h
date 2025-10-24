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
    static bool LoadModel(const VulkanContext* context, const std::string& filedir, const std::string& filename, std::vector<Mesh>& outMesh);
private:
    static void processNode(const VulkanContext* context, aiNode* node, const aiScene* scene, const std::string& filedir, std::vector<Mesh>& outMesh);
    static void processMesh(const VulkanContext* context, aiMesh* mesh, const aiScene* scene, const std::string& filedir, std::vector<Mesh>& outMesh);
};