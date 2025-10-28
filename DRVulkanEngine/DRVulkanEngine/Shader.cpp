#include "Shader.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include "VulkanContext.h"
#include "Vertex.h"

void Shader::initialize(const VulkanContext* inContext, const std::string& inShaderPath)
{
	context_ = inContext;

    auto shaderCode = readFile(inShaderPath);
    module_ = createShaderModule(shaderCode); // createShaderModule이 context를 받는다고 가정

    // --- 2. SPIR-V 리플렉션(Reflection) 시작 ---
    SpvReflectShaderModule reflectModule;
    SpvReflectResult result = spvReflectCreateShaderModule(shaderCode.size(), shaderCode.data(), &reflectModule);
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error("SPIR-V 리플렉션 모듈 생성 실패!");
    }

    // --- 3. 기본 셰이더 스테이지 정보 채우기 ---
    stageInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo_.stage = static_cast<VkShaderStageFlagBits>(reflectModule.shader_stage);
    stageInfo_.module = module_;
    stageInfo_.pName = "main";
    //const SpvReflectEntryPoint* entry_point = spvReflectGetEntryPoint(&reflectModule, "main");

    //if (entry_point) {
    //    stageInfo_.pName = entry_point->name; // 당연히 "main"이 할당됨
    //}
    //else {
    //    // 오류 처리: "main" 진입점을 찾을 수 없음!
    //    throw std::runtime_error("Shader Entry Point 못찾음!!");
    //}

    // --- 4. Descriptor Set Layout 정보 추출 ---
    uint32_t setCount = 0;
    spvReflectEnumerateDescriptorSets(&reflectModule, &setCount, nullptr);
    std::vector<SpvReflectDescriptorSet*> sets(setCount);
    spvReflectEnumerateDescriptorSets(&reflectModule, &setCount, sets.data());
    // VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
    for (const auto* pSet : sets) {
        std::vector<LayoutBindingInfo> bindings;
        for (uint32_t i = 0; i < pSet->binding_count; ++i) {
			LayoutBindingInfo layoutBindingInfo{};
			layoutBindingInfo.resourceName = pSet->bindings[i]->name;

            const auto* pBinding = pSet->bindings[i];
            layoutBindingInfo.bindingInfo.binding = pBinding->binding;
            layoutBindingInfo.bindingInfo.descriptorType = static_cast<VkDescriptorType>(pBinding->descriptor_type);
            layoutBindingInfo.bindingInfo.descriptorCount = pBinding->count;
            layoutBindingInfo.bindingInfo.stageFlags = static_cast<VkShaderStageFlags>(reflectModule.shader_stage);

            bindings.push_back(layoutBindingInfo);
        }
        descriptorSetLayouts_[pSet->set] = bindings;
    }

    // --- 5. Vertex Input Attribute 정보 추출 (Vertex Shader일 경우에만) ---
    //if (reflectModule.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) {
    //    uint32_t varCount = 0;
    //    spvReflectEnumerateInputVariables(&reflectModule, &varCount, nullptr);
    //    std::vector<SpvReflectInterfaceVariable*> inputs(varCount);
    //    spvReflectEnumerateInputVariables(&reflectModule, &varCount, inputs.data());

    //    for (const auto* pVar : inputs) {
    //        // Built-in 변수 (예: gl_VertexIndex)는 제외
    //        if (pVar->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) {
    //            continue;
    //        }
    //        VkVertexInputAttributeDescription attribute{};
    //        attribute.location = pVar->location;
    //        attribute.binding = 0; // ?? 하나의 버텍스 버퍼를 0번 바인딩에 쓴다고 가정
    //        attribute.format = static_cast<VkFormat>(pVar->format);
    //        attribute.offset = 0; // ?? 중요: 이 offset은 나중에 C++ Vertex 구조체를 기준으로 다시 계산해야 함!
    //        inputAttributes_.push_back(attribute);
    //    }
    //}

    if (reflectModule.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) {
        // 리플렉션 방식 대신, C++ Vertex 구조체에 정의된 정보를 직접 가져옵니다.
        // 이 방식은 offset을 정확하게 계산할 수 있어 훨씬 안정적입니다.
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        // Shader 클래스가 파이프라인 생성 시 사용할 수 있도록 내부 멤버 변수에 복사합니다.
        inputAttributes_.assign(attributeDescriptions.begin(), attributeDescriptions.end());
    }

    // --- 6. Push Constant 정보 추출 ---
    uint32_t blockCount = 0;
    spvReflectEnumeratePushConstantBlocks(&reflectModule, &blockCount, nullptr);
    std::vector<SpvReflectBlockVariable*> blocks(blockCount);
    spvReflectEnumeratePushConstantBlocks(&reflectModule, &blockCount, blocks.data());

    for (const auto* pBlock : blocks) {
        VkPushConstantRange range{};
        range.stageFlags = static_cast<VkShaderStageFlags>(reflectModule.shader_stage);
        range.offset = pBlock->offset;
        range.size = pBlock->size;
        pushConstantRanges_.push_back(range);
    }

    // --- 7. 리플렉션 모듈 정리 ---
    spvReflectDestroyShaderModule(&reflectModule);
}

// Shader 객체가 파괴될 때 모듈도 파괴해야 합니다.
void Shader::destroy(VkDevice device) {
    if (module_ != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device, module_, nullptr);
    }
}

std::vector<char> Shader::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file: " + filename);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}


VkShaderModule Shader::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(context_->getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}
