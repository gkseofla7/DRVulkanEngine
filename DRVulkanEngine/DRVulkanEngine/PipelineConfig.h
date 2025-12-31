#pragma once
#include <vulkan/vulkan.h>
#include <string>
struct PipelineConfig {
    std::string pipelineName;
    std::string vertexShaderPath;
    std::string fragmentShaderPath;

    VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS; // �⺻���� LESS
    VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT; // �⺻���� BACK

    bool blendEnable = VK_FALSE;
    // ����� ��� ���� (�Ϲ����� ���� ����� �⺻��)
    VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

    // ���� �׽�Ʈ Ȱ��ȭ ����
    bool depthTestEnable = VK_TRUE;
    // ���� ���� ���� Ȱ��ȭ ����
    bool depthWriteEnable = VK_TRUE;

    bool useVertexInput = true;

    VkFormat colorAttachmentFormat;
    VkFormat depthAttachmentFormat;
};
