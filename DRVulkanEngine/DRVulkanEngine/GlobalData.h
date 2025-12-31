#pragma once
#define USE_BDA_BUFFER 1
#define USE_GENERAL_LAYOUT 1
struct PushConstantData {
    VkDeviceAddress boneAddress = 0;
    int modelUBIndex = -1;
    int materialIndex = -1;

    int boneUbIndex = -1;
    int padding = 0;
};