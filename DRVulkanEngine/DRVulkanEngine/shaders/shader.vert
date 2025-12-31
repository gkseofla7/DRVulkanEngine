#version 450
#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_EXT_buffer_reference : enable

#define MAX_OBJECTS 128
#define MAX_BONES 100

layout(constant_id = 0) const bool USE_BDA_BUFFER = false; 

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
layout(location = 5) in ivec4 inBoneIDs;
layout(location = 6) in vec4 inWeights;

layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out mat3 fragTBN;

layout(push_constant) uniform PushConstants {
    uint64_t boneAddress;
    int modelUBIndex;
    int materialIndex;
    int boneUbIndex;
    int padding;
} pc;

layout(std140, set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo[MAX_OBJECTS];


layout(buffer_reference, std430) readonly restrict buffer BonePtr {
    mat4 finalBoneMatrix[];
};
layout(std140, set = 0, binding = 1) uniform BoneMatrices {
    mat4 finalBones[MAX_BONES];
} boneData[MAX_OBJECTS];





void main() {
    mat4 currentModelMatrix = ubo[pc.modelUBIndex].model;
    mat4 currentViewMatrix = ubo[pc.modelUBIndex].view;
    mat4 currentProjMatrix = ubo[pc.modelUBIndex].proj;

    mat4 totalBoneTransform = mat4(1.0f);
    if (inWeights.x > 0.0) {
        
        totalBoneTransform = mat4(0.0f); 
        
        for(int i = 0; i < 4; i++) {
            if(inBoneIDs[i] < 0 || inWeights[i] == 0.0) {
                continue;
            }
            
            mat4 boneMatrix;
            if(USE_BDA_BUFFER)
            {
                BonePtr bones = BonePtr(pc.boneAddress);
                boneMatrix = bones.finalBoneMatrix[inBoneIDs[i]];
            }
            else
            {
                boneMatrix = boneData[pc.boneUbIndex].finalBones[inBoneIDs[i]];
            }
            totalBoneTransform += boneMatrix * inWeights[i];
        }
    }
    
    vec4 animatedPos = totalBoneTransform * vec4(inPosition, 1.0);
    vec4 worldPos = currentModelMatrix * animatedPos;
    fragWorldPos = worldPos.xyz;
    gl_Position = currentProjMatrix * currentViewMatrix * worldPos;

    mat3 boneTransformMat3 = mat3(totalBoneTransform);
    vec3 T = normalize(mat3(currentModelMatrix) * (boneTransformMat3 * inTangent));
    vec3 B = normalize(mat3(currentModelMatrix) * (boneTransformMat3 * inBitangent));
    vec3 N = normalize(mat3(currentModelMatrix) * (boneTransformMat3 * inNormal));
    fragTBN = mat3(T, B, N);
    fragNormal = N;

    fragTexCoord = inTexCoord;
}