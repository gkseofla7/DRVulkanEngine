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
    // 생성자: Vulkan Context와 정점/인덱스 데이터를 받습니다.
    Model(const VulkanContext* context, const std::string& filedir, const std::string& filename);
    ~Model();

    Model(const Model& other) = delete;
    Model& operator=(const Model& other) = delete;

    // 2. 이동 생성자와 이동 대입 연산자는 컴파일러가 자동으로 생성하도록 하거나,
    //    필요하다면 직접 구현할 수 있습니다. (기본으로 둬도 보통 괜찮습니다)
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

