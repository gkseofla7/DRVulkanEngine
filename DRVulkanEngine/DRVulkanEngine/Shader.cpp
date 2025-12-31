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
    module_ = createShaderModule(shaderCode); // createShaderModule�� context�� �޴´ٰ� ����

    // --- 2. SPIR-V ���÷���(Reflection) ���� ---
    SpvReflectShaderModule reflectModule;
    SpvReflectResult result = spvReflectCreateShaderModule(shaderCode.size(), shaderCode.data(), &reflectModule);
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error("SPIR-V ���÷��� ��� ���� ����!");
    }

    // --- 3. �⺻ ���̴� �������� ���� ä��� ---
    stageInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo_.stage = static_cast<VkShaderStageFlagBits>(reflectModule.shader_stage);
    stageInfo_.module = module_;
    stageInfo_.pName = "main";
    //const SpvReflectEntryPoint* entry_point = spvReflectGetEntryPoint(&reflectModule, "main");

    //if (entry_point) {
    //    stageInfo_.pName = entry_point->name; // �翬�� "main"�� �Ҵ��
    //}
    //else {
    //    // ���� ó��: "main" �������� ã�� �� ����!
    //    throw std::runtime_error("Shader Entry Point ��ã��!!");
    //}

    // --- 4. Descriptor Set Layout ���� ���� ---
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

    // --- 5. Vertex Input Attribute ���� ���� (Vertex Shader�� ��쿡��) ---
    //if (reflectModule.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) {
    //    uint32_t varCount = 0;
    //    spvReflectEnumerateInputVariables(&reflectModule, &varCount, nullptr);
    //    std::vector<SpvReflectInterfaceVariable*> inputs(varCount);
    //    spvReflectEnumerateInputVariables(&reflectModule, &varCount, inputs.data());

    //    for (const auto* pVar : inputs) {
    //        // Built-in ���� (��: gl_VertexIndex)�� ����
    //        if (pVar->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) {
    //            continue;
    //        }
    //        VkVertexInputAttributeDescription attribute{};
    //        attribute.location = pVar->location;
    //        attribute.binding = 0; // ?? �ϳ��� ���ؽ� ���۸� 0�� ���ε��� ���ٰ� ����
    //        attribute.format = static_cast<VkFormat>(pVar->format);
    //        attribute.offset = 0; // ?? �߿�: �� offset�� ���߿� C++ Vertex ����ü�� �������� �ٽ� ����ؾ� ��!
    //        inputAttributes_.push_back(attribute);
    //    }
    //}

    if (reflectModule.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) {
        // ���÷��� ��� ���, C++ Vertex ����ü�� ���ǵ� ������ ���� �����ɴϴ�.
        // �� ����� offset�� ��Ȯ�ϰ� ����� �� �־� �ξ� �������Դϴ�.
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        // Shader Ŭ������ ���������� ���� �� ����� �� �ֵ��� ���� ��� ������ �����մϴ�.
        inputAttributes_.assign(attributeDescriptions.begin(), attributeDescriptions.end());
    }

    // --- 6. Push Constant ���� ���� ---
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

    // --- 7. ���÷��� ��� ���� ---
    spvReflectDestroyShaderModule(&reflectModule);
}

// Shader ��ü�� �ı��� �� ��⵵ �ı��ؾ� �մϴ�.
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
