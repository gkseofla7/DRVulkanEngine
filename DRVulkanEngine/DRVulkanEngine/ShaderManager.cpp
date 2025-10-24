#include "ShaderManager.h"
#include "VulkanContext.h"
#include <iostream>

void ShaderManager::initialize(const VulkanContext* context) {
    context_ = context;
}

void ShaderManager::destroy() {
    // VulkanContext�κ��� VkDevice�� �����´ٰ� ����
    VkDevice device = context_->getDevice();

    // �����ϴ� ��� ���̴� ����� �ı�
    for (auto const& [path, shader] : shaders_) {
        shader->destroy(device);
    }
    // ���� ����� unique_ptr�� �ڵ����� �޸� �����ϵ��� ��
    shaders_.clear();
}

Shader* ShaderManager::getShader(const std::string& shaderPath) {
    // 1. �ʿ��� �ش� ����� ���̴��� �̹� �ε�Ǿ����� Ȯ��
    auto it = shaders_.find(shaderPath);
    if (it != shaders_.end()) {
        // �̹� �ִٸ� �ٷ� �����͸� ��ȯ
        return it->second.get();
    }

    // 2. �ʿ� ���ٸ� ���� �ε�
    std::cout << "Loading shader: " << shaderPath << std::endl;

    // ���ο� Shader ��ü�� unique_ptr�� ����
    auto newShader = std::make_unique<Shader>();

    // Shader �ʱ�ȭ (���� �б�, ��� ����, ���÷��� ��)
    // ���� ������Ʈ������ initialize�� ������ ��츦 ����� bool�� ��ȯ�޾� ó���ϴ� ���� �����ϴ�.
    newShader->initialize(context_, shaderPath);

    // �ε��� ���̴��� �����͸� �̸� ����
    Shader* shaderPtr = newShader.get();

    // �ʿ� ���� ������ ���̴��� �߰�
    // std::move�� ���� unique_ptr�� �������� ������ ����
    shaders_[shaderPath] = std::move(newShader);

    return shaderPtr;
}