#pragma once
#include <vulkan/vulkan.h>
#include <string>
struct PipelineConfig {
    std::string pipelineName;
    std::string vertexShaderPath;
    std::string fragmentShaderPath;

    VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS; // 기본값은 LESS
    VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT; // 기본값은 BACK

    bool blendEnable = VK_FALSE;
    // 블렌딩 방식 설정 (일반적인 알파 블렌딩 기본값)
    VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

    // 깊이 테스트 활성화 여부
    bool depthTestEnable = VK_TRUE;
    // 깊이 버퍼 쓰기 활성화 여부
    bool depthWriteEnable = VK_TRUE;

    bool useVertexInput = true;

    VkFormat colorAttachmentFormat;
    VkFormat depthAttachmentFormat;
};
