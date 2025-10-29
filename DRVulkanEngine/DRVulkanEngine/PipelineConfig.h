#pragma once
#include <vulkan/vulkan.h>
#include <string>
struct PipelineConfig {
    std::string pipelineName;
    std::string vertexShaderPath;
    std::string fragmentShaderPath;

    VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS; // 기본값은 LESS
    VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT; // 기본값은 BACK
};
