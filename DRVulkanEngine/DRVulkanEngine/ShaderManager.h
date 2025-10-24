// ShaderManager.h
#pragma once

#include <string>
#include <map>
#include <memory>
#include "Shader.h"

class VulkanContext;

class ShaderManager {
public:
    ShaderManager() = default;
    ~ShaderManager() = default;
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    void initialize(const VulkanContext* context);
    void destroy();

    Shader* getShader(const std::string& shaderPath);
private:
    std::map<std::string, std::unique_ptr<Shader>> shaders_;
    const VulkanContext* context_ = nullptr;
};