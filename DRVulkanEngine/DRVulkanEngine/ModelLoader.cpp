#include "ModelLoader.h"
#include "VulkanContext.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include "Texture.h"

// --- Public API ---
std::unique_ptr<Model> ModelLoader::LoadModel(const VulkanContext* context, const std::string& filedir, const std::string& filename) {
    Assimp::Importer importer;

    // --- �� �κ��� �߰�/������ �κ��Դϴ� ---
    // 1. filedir�� filename�� ���� ��ü ���� ��θ� �����մϴ�.
    //    ��� �����ڴ� '/'�� ����ϴ� ���� ���� �ü������ ȣȯ���� �����ϴ�.
    std::string filepath = filedir + "/" + filename;
    // --- ���� �� ---

    // Assimp �÷���: ���� �ε��� �� ������ ��ó�� �ܰ��
    const unsigned int flags = aiProcess_Triangulate        // ��� ���� �ﰢ������
        | aiProcess_FlipUVs            // �ؽ�ó Y�� ������
        | aiProcess_GenSmoothNormals   // �ε巯�� ���� ���� ����
        | aiProcess_CalcTangentSpace;  // ź��Ʈ/����ź��Ʈ ����

    // 2. ������ filepath ������ ����Ͽ� ������ �н��ϴ�.
    const aiScene* scene = importer.ReadFile(filepath, flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ASSIMP ����: " << importer.GetErrorString() << std::endl;
        return nullptr;
    }

    // Model ��ü�� ���� �����ϰ�, processNode�� ���� ������ ä��ϴ�.
    auto model = std::make_unique<Model>(context);

    processNode(context, scene->mRootNode, scene, filedir, *model);

    return model;
}

// --- Private Helper Functions ---

void ModelLoader::processNode(const VulkanContext* context, aiNode* node, const aiScene* scene, const std::string& filedir, Model& outModel) {
    // 1. ���� ��忡 ���� ��� �޽��� ó���մϴ�.
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        outModel.addMesh(processMesh(context, mesh, scene, filedir));
    }

    // 2. ���� ����� ��� �ڽ� ��忡 ���� ��������� �� �Լ��� ȣ���մϴ�.
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(context, node->mChildren[i], scene, filedir, outModel);
    }
}

Mesh ModelLoader::processMesh(const VulkanContext* context, aiMesh* mesh, const aiScene* scene, const std::string& filedir) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices; // �� ���� ������ �����ϱ� ���� uint32_t�� ����
    //std::vector<Texture> textures; // (����) ����/�ؽ�ó ������

    // 1. ����(Vertex) ������ ����
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex{}; // ���� �ʱ�ȭ

        // ��ġ (Position)
        vertex.pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

        // ���� (Normal)
        if (mesh->HasNormals()) {
            vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
        }

        // �ؽ�ó ��ǥ (Texture Coordinates)
        if (mesh->mTextureCoords[0]) { // ù ��° UV ä���� �ִ��� Ȯ��
            vertex.texCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        }

        if (mesh->HasTangentsAndBitangents()) {
            // ź��Ʈ
            vertex.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };

            // ����ź��Ʈ (���̴����� ����� ���̹Ƿ� ���⼭�� �ּ� ó��)
            // ���� C++���� �Ѱ��ְ� �ʹٸ� Vertex ����ü�� bitangent ����� �߰��ϰ� �ּ��� �����ϼ���.
            vertex.bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
        }

        vertices.push_back(vertex);
    }

    // 2. �ε���(Index) ������ ����
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }
    std::shared_ptr<Texture> diffuseTex;
    std::shared_ptr<Texture> specularTex;
    std::shared_ptr<Texture> normalTex;
    std::shared_ptr<Texture> ambientTex;
    std::shared_ptr<Texture> emissiveTex;
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        auto loadMaterialTexture = [&](aiTextureType type) -> std::shared_ptr<Texture> {
            if (material->GetTextureCount(type) > 0) {
                aiString texturePathInModel;
                // �׻� ù ��° �ؽ�ó�� �����´ٰ� �����մϴ�.
                material->GetTexture(type, 0, &texturePathInModel);

                // Assimp�� �˷��� �ؽ�ó ��ο��� ���� �̸��� �����մϴ�.
                std::string textureFilename = std::string(texturePathInModel.C_Str());
                size_t lastSeparator = textureFilename.find_last_of("/\\");
                if (std::string::npos != lastSeparator) {
                    textureFilename = textureFilename.substr(lastSeparator + 1);
                }

                std::string finalTexturePath = filedir + "/" + textureFilename;

                return std::make_shared<Texture>(context, finalTexturePath.c_str());
            }
            // �ش� Ÿ���� �ؽ�ó�� ������ nullptr ��ȯ
            return nullptr;
            };

        diffuseTex  = loadMaterialTexture(aiTextureType_DIFFUSE);
        specularTex = loadMaterialTexture(aiTextureType_SPECULAR);

        normalTex = loadMaterialTexture(aiTextureType_NORMALS);
        if (!normalTex) {
            normalTex = loadMaterialTexture(aiTextureType_HEIGHT);
        }
        ambientTex = loadMaterialTexture(aiTextureType_AMBIENT);
        emissiveTex = loadMaterialTexture(aiTextureType_EMISSIVE);
    }
    Mesh ret = {};
    ret.initialize(context, vertices, indices,
        diffuseTex,
        specularTex,
        normalTex,
        ambientTex,
		emissiveTex);
    return ret;
}
