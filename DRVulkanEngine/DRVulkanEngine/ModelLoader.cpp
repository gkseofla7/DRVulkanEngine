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

    // --- 이 부분이 추가/수정된 부분입니다 ---
    // 1. filedir와 filename을 합쳐 전체 파일 경로를 생성합니다.
    //    경로 구분자는 '/'를 사용하는 것이 여러 운영체제에서 호환성이 좋습니다.
    std::string filepath = filedir + "/" + filename;
    // --- 수정 끝 ---

    // Assimp 플래그: 모델을 로드할 때 적용할 후처리 단계들
    const unsigned int flags = aiProcess_Triangulate        // 모든 면을 삼각형으로
        | aiProcess_FlipUVs            // 텍스처 Y축 뒤집기
        | aiProcess_GenSmoothNormals   // 부드러운 법선 벡터 생성
        | aiProcess_CalcTangentSpace;  // 탄젠트/바이탄젠트 생성

    // 2. 합쳐진 filepath 변수를 사용하여 파일을 읽습니다.
    const aiScene* scene = importer.ReadFile(filepath, flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ASSIMP 에러: " << importer.GetErrorString() << std::endl;
        return nullptr;
    }

    // Model 객체를 먼저 생성하고, processNode를 통해 내용을 채웁니다.
    auto model = std::make_unique<Model>(context);

    processNode(context, scene->mRootNode, scene, filedir, *model);

    return model;
}

// --- Private Helper Functions ---

void ModelLoader::processNode(const VulkanContext* context, aiNode* node, const aiScene* scene, const std::string& filedir, Model& outModel) {
    // 1. 현재 노드에 속한 모든 메쉬를 처리합니다.
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        outModel.addMesh(processMesh(context, mesh, scene, filedir));
    }

    // 2. 현재 노드의 모든 자식 노드에 대해 재귀적으로 이 함수를 호출합니다.
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(context, node->mChildren[i], scene, filedir, outModel);
    }
}

Mesh ModelLoader::processMesh(const VulkanContext* context, aiMesh* mesh, const aiScene* scene, const std::string& filedir) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices; // 더 많은 정점을 지원하기 위해 uint32_t가 좋음
    //std::vector<Texture> textures; // (향후) 재질/텍스처 데이터

    // 1. 정점(Vertex) 데이터 추출
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex{}; // 제로 초기화

        // 위치 (Position)
        vertex.pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

        // 법선 (Normal)
        if (mesh->HasNormals()) {
            vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
        }

        // 텍스처 좌표 (Texture Coordinates)
        if (mesh->mTextureCoords[0]) { // 첫 번째 UV 채널이 있는지 확인
            vertex.texCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        }

        if (mesh->HasTangentsAndBitangents()) {
            // 탄젠트
            vertex.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };

            // 바이탄젠트 (셰이더에서 계산할 것이므로 여기서는 주석 처리)
            // 만약 C++에서 넘겨주고 싶다면 Vertex 구조체에 bitangent 멤버를 추가하고 주석을 해제하세요.
            vertex.bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
        }

        vertices.push_back(vertex);
    }

    // 2. 인덱스(Index) 데이터 추출
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
                // 항상 첫 번째 텍스처를 가져온다고 가정합니다.
                material->GetTexture(type, 0, &texturePathInModel);

                // Assimp이 알려준 텍스처 경로에서 파일 이름만 추출합니다.
                std::string textureFilename = std::string(texturePathInModel.C_Str());
                size_t lastSeparator = textureFilename.find_last_of("/\\");
                if (std::string::npos != lastSeparator) {
                    textureFilename = textureFilename.substr(lastSeparator + 1);
                }

                std::string finalTexturePath = filedir + "/" + textureFilename;

                return std::make_shared<Texture>(context, finalTexturePath.c_str());
            }
            // 해당 타입의 텍스처가 없으면 nullptr 반환
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
