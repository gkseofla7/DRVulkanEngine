#include "ModelLoader.h"
#include "VulkanContext.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include "Texture.h"
#include <glm/gtc/type_ptr.hpp> // glm::make_mat4를 위해 추가

// static 멤버 변수 정의
std::map<std::string, BoneInfo> ModelLoader::boneInfoMap_;
int ModelLoader::boneCounter_ = 0;
glm::mat4 ModelLoader::globalInverseTransform_;
// --- 1. 스켈레톤과 메시 로딩 함수 ---
bool ModelLoader::LoadSkinnedModel(const VulkanContext* context, const std::string& filedir, const std::string& filename, std::vector<Mesh>& outMesh) {
    Assimp::Importer importer;
    std::string filepath = filedir + "/" + filename;

    // 뼈대 구조를 정리하고 Vulkan에 맞게 데이터를 준비하는 플래그들
    const unsigned int flags = aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_OptimizeGraph;

    const aiScene* scene = importer.ReadFile(filepath, flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ASSIMP 에러 (모델 로딩): " << importer.GetErrorString() << std::endl;
        return false;
    }


    const auto& rootTransformAssimp = scene->mRootNode->mTransformation;

    // 2. GLM의 Column-Major 행렬로 올바르게 변환(전치)합니다.
    glm::mat4 globalTransform = glm::mat4(
        rootTransformAssimp.a1, rootTransformAssimp.b1, rootTransformAssimp.c1, rootTransformAssimp.d1,
        rootTransformAssimp.a2, rootTransformAssimp.b2, rootTransformAssimp.c2, rootTransformAssimp.d2,
        rootTransformAssimp.a3, rootTransformAssimp.b3, rootTransformAssimp.c3, rootTransformAssimp.d3,
        rootTransformAssimp.a4, rootTransformAssimp.b4, rootTransformAssimp.c4, rootTransformAssimp.d4
    );

    // 3. 그 행렬의 역행렬을 계산하여 멤버 변수에 저장합니다.
    //    이것이 '기울어진 방'을 바로 펴는 마법의 행렬입니다.
    globalInverseTransform_ = glm::inverse(globalTransform);
    // 재귀적으로 노드를 순회하며 메시와 뼈 정보를 처리합니다.
    // 이 과정에서 static 멤버인 boneInfoMap_이 채워집니다.
    processNode(context, scene->mRootNode, scene, filedir, outMesh);

    return true;
}

// --- 2. 애니메이션 로딩 함수 ---
bool ModelLoader::LoadAnimations(const VulkanContext* context, const std::string& filedir, const std::string& filename, std::vector<Animation>& outAnimations) {
    Assimp::Importer importer; // 함수 내에서 생명주기 관리
    std::string filepath = filedir + "/" + filename;

    // 애니메이션 로딩 시에는 많은 후처리 과정이 필요 없습니다.
    const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate);

    if (!scene || !scene->HasAnimations()) {
        std::cerr << "ASSIMP 에러 (애니메이션 로딩 또는 애니메이션 없음): " << importer.GetErrorString() << std::endl;
        return false;
    }

    // 로드된 씬에서 애니메이션 데이터를 추출합니다.
    // Animation 클래스가 생성될 때 필요한 모든 데이터를 복사해야 합니다.
    processAnimations(scene, outAnimations);

    // importer가 여기서 소멸되면서 scene 메모리도 자동으로 해제됩니다.
    return true;
}

// --- 헬퍼 함수들 ---

void ModelLoader::processNode(const VulkanContext* context, aiNode* node, const aiScene* scene, const std::string& filedir, std::vector<Mesh>& outMesh) {
    // 1. 현재 노드에 속한 모든 메쉬를 처리합니다.
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(context, mesh, scene, filedir, outMesh);
    }

    // 2. 현재 노드의 모든 자식 노드에 대해 재귀적으로 이 함수를 호출합니다.
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(context, node->mChildren[i], scene, filedir, outMesh);
    }
}

void ModelLoader::processMesh(const VulkanContext* context, aiMesh* mesh, const aiScene* scene, const std::string& filedir, std::vector<Mesh>& outMeshes) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // 1. 정점(Vertex) 데이터 추출
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex{};

        for (int j = 0; j < MAX_BONE_INFLUENCE; j++) {
            vertex.boneIDs[j] = -1;
            vertex.weights[j] = 0.0f;
        }

        vertex.pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

        if (mesh->HasNormals()) {
            vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
        }

        if (mesh->mTextureCoords[0]) {
            vertex.texCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        }

        if (mesh->HasTangentsAndBitangents()) {
            vertex.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
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

    // 3. 뼈 가중치 데이터 추출
    extractBoneWeightForVertices(vertices, mesh, scene);

    // 4. 재질 및 텍스처 데이터 추출
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
                material->GetTexture(type, 0, &texturePathInModel);

                std::string textureFilename = std::string(texturePathInModel.C_Str());
                size_t lastSeparator = textureFilename.find_last_of("/\\");
                if (std::string::npos != lastSeparator) {
                    textureFilename = textureFilename.substr(lastSeparator + 1);
                }

                std::string finalTexturePath = filedir + "/" + textureFilename;
                return std::make_shared<Texture>(context, finalTexturePath.c_str());
            }
            return nullptr;
            };

        diffuseTex = loadMaterialTexture(aiTextureType_DIFFUSE);
        specularTex = loadMaterialTexture(aiTextureType_SPECULAR);
        normalTex = loadMaterialTexture(aiTextureType_NORMALS);
        if (!normalTex) {
            normalTex = loadMaterialTexture(aiTextureType_HEIGHT);
        }
        ambientTex = loadMaterialTexture(aiTextureType_AMBIENT);
        emissiveTex = loadMaterialTexture(aiTextureType_EMISSIVE);
    }

    outMeshes.emplace_back();
    Mesh& newMeshRef = outMeshes.back();
    newMeshRef.initialize(context, vertices, indices,
        diffuseTex, specularTex, normalTex, ambientTex, emissiveTex);
}

void ModelLoader::setVertexBoneData(Vertex& vertex, int boneID, float weight) {
    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
        if (vertex.boneIDs[i] < 0) {
            vertex.weights[i] = weight;
            vertex.boneIDs[i] = boneID;
            break;
        }
    }
}

void ModelLoader::extractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene) {
    for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();

        if (boneInfoMap_.find(boneName) == boneInfoMap_.end()) {
            BoneInfo newBoneInfo;
            newBoneInfo.id = boneCounter_;

            const auto& mat = mesh->mBones[boneIndex]->mOffsetMatrix;

            // Assimp(Row-Major) -> GLM(Column-Major) 변환 (암시적 전치)
            newBoneInfo.offsetMatrix = glm::mat4(
                mat.a1, mat.b1, mat.c1, mat.d1,
                mat.a2, mat.b2, mat.c2, mat.d2,
                mat.a3, mat.b3, mat.c3, mat.d3,
                mat.a4, mat.b4, mat.c4, mat.d4
            );

            boneInfoMap_[boneName] = newBoneInfo;
            boneID = boneCounter_;
            boneCounter_++;
        }
        else {
            boneID = boneInfoMap_[boneName].id;
        }

        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;
            if (vertexId >= vertices.size()) continue;
            setVertexBoneData(vertices[vertexId], boneID, weight);
        }
    }
}

void ModelLoader::processAnimations(const aiScene* scene, std::vector<Animation>& outAnimations) {
    if (!scene->HasAnimations()) {
        return;
    }

    for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
        aiAnimation* animation = scene->mAnimations[i];
        
        // 올바른 BoneInfo 맵을 전달
        outAnimations.emplace_back(Animation(scene, animation, boneInfoMap_));
    }
}