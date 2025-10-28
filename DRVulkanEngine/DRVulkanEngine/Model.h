#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include "Mesh.h"
#include "Animation.h"
#include "Animator.h"
class VulkanContext;
class UniformBuffer;
class TextureArray;
class UniformBufferArray;
class Resource; 
#define MAX_BONES 100 
struct UniformBufferBone {
    glm::mat4 finalBoneMatrix[MAX_BONES];
};

struct UniformBufferObject {
    glm::mat4 world;
    glm::mat4 view;
    glm::mat4 proj;
};

class Model
{
public:
    Model(const VulkanContext* context, const std::string& filedir, const std::string& filename);
    ~Model();

    Model(const Model& other) = delete;
    Model& operator=(const Model& other) = delete;
    Model(Model&& other) noexcept = default;
    Model& operator=(Model&& other) noexcept = default;

    void prepareBindless(UniformBufferArray& modelUbArray, UniformBufferArray& materialUbArray, UniformBufferArray& boneUbArray, TextureArray& textures);
	void updateUniformBuffer(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projMatrix);

    void addMesh(Mesh&& mesh);


    void update(float deltaTime);
    void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);

private:
    const VulkanContext* context_;
    std::vector<Mesh> meshes_;

	std::unique_ptr<class UniformBuffer> modelUB_;

	int modelUbIndex_ = -1;
	int boneUbIndex_ = -1;

    std::unique_ptr<Animator> animator_;
	std::vector<Animation> animations_;

    std::unique_ptr<class UniformBuffer> boneUB_;

};

