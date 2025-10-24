#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <map>
#include "Mesh.h"
class VulkanContext;
class UniformBuffer;

struct UniformBufferObject {
    glm::mat4 world;
    glm::mat4 view;
    glm::mat4 proj;
};

class Model
{
public:
    // ������: Vulkan Context�� ����/�ε��� �����͸� �޽��ϴ�.
    Model(const VulkanContext* context, const std::string& filedir, const std::string& filename);
    ~Model();

    Model(const Model& other) = delete;
    Model& operator=(const Model& other) = delete;

    // 2. �̵� �����ڿ� �̵� ���� �����ڴ� �����Ϸ��� �ڵ����� �����ϵ��� �ϰų�,
    //    �ʿ��ϴٸ� ���� ������ �� �ֽ��ϴ�. (�⺻���� �ֵ� ���� �������ϴ�)
    Model(Model&& other) noexcept = default;
    Model& operator=(Model&& other) noexcept = default;

    void prepareBindless(std::map<std::string, UniformBuffer*>& uniformBuffers_, std::map<std::string, Texture*>& textures_);
	void updateUniformBuffer(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projMatrix);

    void addMesh(Mesh&& mesh);
    void draw(VkCommandBuffer commandBuffer);

private:
    const VulkanContext* context_;
    std::vector<Mesh> meshes_;

	std::unique_ptr<class UniformBuffer> modelUB_;
};

