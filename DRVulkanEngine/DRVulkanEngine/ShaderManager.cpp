#include "ShaderManager.h"
#include "VulkanContext.h"
#include <iostream>

void ShaderManager::initialize(const VulkanContext* context) {
    context_ = context;
}

void ShaderManager::destroy() {
    // VulkanContext로부터 VkDevice를 가져온다고 가정
    VkDevice device = context_->getDevice();

    // 관리하던 모든 셰이더 모듈을 파괴
    for (auto const& [path, shader] : shaders_) {
        shader->destroy(device);
    }
    // 맵을 비워서 unique_ptr가 자동으로 메모리 해제하도록 함
    shaders_.clear();
}

Shader* ShaderManager::getShader(const std::string& shaderPath) {
    // 1. 맵에서 해당 경로의 셰이더가 이미 로드되었는지 확인
    auto it = shaders_.find(shaderPath);
    if (it != shaders_.end()) {
        // 이미 있다면 바로 포인터를 반환
        return it->second.get();
    }

    // 2. 맵에 없다면 새로 로드
    std::cout << "Loading shader: " << shaderPath << std::endl;

    // 새로운 Shader 객체를 unique_ptr로 생성
    auto newShader = std::make_unique<Shader>();

    // Shader 초기화 (파일 읽기, 모듈 생성, 리플렉션 등)
    // 실제 프로젝트에서는 initialize가 실패할 경우를 대비해 bool을 반환받아 처리하는 것이 좋습니다.
    newShader->initialize(context_, shaderPath);

    // 로드한 셰이더의 포인터를 미리 저장
    Shader* shaderPtr = newShader.get();

    // 맵에 새로 생성한 셰이더를 추가
    // std::move를 통해 unique_ptr의 소유권을 맵으로 이전
    shaders_[shaderPath] = std::move(newShader);

    return shaderPtr;
}