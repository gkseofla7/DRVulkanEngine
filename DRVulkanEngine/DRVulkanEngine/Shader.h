#pragma once
#include <vulkan/vulkan.h>
#include <spirv-reflect/spirv_reflect.h>
#include <vector>
#include <string>
#include <map>

struct LayoutBindingInfo
{
    std::string resourceName;
    VkDescriptorSetLayoutBinding bindingInfo;
};

class VulkanContext;
class Shader {
public:
	void initialize(const VulkanContext* inContext, const std::string& inShaderPath);
    void destroy(VkDevice device);
private:
    std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);
public:
    VkPipelineShaderStageCreateInfo stageInfo_;
    std::map<uint32_t, std::vector<LayoutBindingInfo>> descriptorSetLayouts_;
    std::vector<VkVertexInputAttributeDescription> inputAttributes_;
    std::vector<VkPushConstantRange> pushConstantRanges_;

    VkShaderModule module_;
    SpvReflectShaderModule reflectionModule_;

	const VulkanContext* context_;
};