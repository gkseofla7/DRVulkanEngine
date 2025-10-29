#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include "Mesh.h"
#include "Animation.h"
#include "Animator.h"
#include "ModelConfig.h"
#include "GlobalData.h"

class VulkanContext;
class UniformBuffer;
class TextureArray;
class UniformBufferArray;
class Resource; 
#define MAX_BONES 100 
struct UniformBufferBone {
    alignas(16) glm::mat4 finalBoneMatrix[MAX_BONES]; // 메모리 정렬 보장
};

struct UniformBufferObject {
    alignas(16) glm::mat4 world;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class Model
{
public:
    Model(const VulkanContext* context, const ModelConfig& modelConfig);
    ~Model();

    Model(const Model& other) = delete;
    Model& operator=(const Model& other) = delete;
    Model(Model&& other) noexcept = default;
    Model& operator=(Model&& other) noexcept = default;

    void prepareBindless(UniformBufferArray& modelUbArray, UniformBufferArray& materialUbArray, UniformBufferArray& boneUbArray, TextureArray& textures);
	void updateUniformBuffer(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projMatrix);

    void addMesh(Mesh&& mesh);

    void update(float deltaTime);
    void draw(VkCommandBuffer commandBuffer);

    void getPushConstantData(PushConstantData& outPushData);
private:
    const VulkanContext* context_;
    std::vector<Mesh> meshes_;

	std::unique_ptr<class UniformBuffer> modelUB_;

	int modelUbIndex_ = -1;
	int boneUbIndex_ = -1;

    std::unique_ptr<Animator> animator_;
	std::vector<Animation> animations_;

    std::unique_ptr<class UniformBuffer> boneUB_;

    bool boneDataDirty_ = true;
    std::vector<glm::mat4> cachedBoneMatrices_;
    uint32_t framesSinceLastBoneUpdate_ = 0;
    
    static constexpr uint32_t BONE_UPDATE_FREQUENCY = 1;
    static constexpr float MATRIX_COMPARISON_THRESHOLD = 0.00001f;
    
    mutable UniformBufferBone ubBoneBuffer_; // 스택 할당 방지를 위한 재사용 버퍼
    
    inline bool isMatrixChanged(const glm::mat4& a, const glm::mat4& b) const;

    ModelConfig modelConfig_;
};

