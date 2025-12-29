#version 450
#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_EXT_buffer_reference : enable

#define MAX_OBJECTS 128
#define MAX_BONES 100 // 최대 뼈 개수 (Animator와 일치시켜야 함)

layout(constant_id = 0) const bool USE_BDA_BUFFER = false; 

// --- 입력(in) 변수 ---
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
layout(location = 5) in ivec4 inBoneIDs;
layout(location = 6) in vec4 inWeights;

// --- 출력(out) 변수 (기존과 동일) ---
layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out mat3 fragTBN;

// --- 유니폼 및 푸시 상수 ---
layout(push_constant) uniform PushConstants {
    uint64_t boneAddress;
    int modelUBIndex;
    int materialIndex;
    int boneUbIndex;
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
    // 1. 현재 객체의 MVP 행렬을 가져옵니다. (기존과 동일)
    mat4 currentModelMatrix = ubo[pc.modelUBIndex].model;
    mat4 currentViewMatrix = ubo[pc.modelUBIndex].view;
    mat4 currentProjMatrix = ubo[pc.modelUBIndex].proj;

    mat4 totalBoneTransform = mat4(1.0f);
    if (inWeights.x > 0.0) {
        
        // 3. 스키닝이 필요하므로, 이제 '가중치 합' 계산을 위해 0 행렬로 리셋합니다.
        totalBoneTransform = mat4(0.0f); 
        
        for(int i = 0; i < 4; i++) {
            // ID가 유효하고 가중치가 있는 뼈만 계산에 포함합니다.
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
    
    // 3. 최종 위치 계산 (뼈 변환을 먼저 적용)
    vec4 animatedPos = totalBoneTransform * vec4(inPosition, 1.0);
    vec4 worldPos = currentModelMatrix * animatedPos;
    fragWorldPos = worldPos.xyz;
    gl_Position = currentProjMatrix * currentViewMatrix * worldPos;

    // 4. 법선, 탄젠트 등 방향 벡터에도 동일한 뼈 변환을 적용합니다.
    // (mat3으로 캐스팅하여 위치 이동(translation) 정보는 제외)
    mat3 boneTransformMat3 = mat3(totalBoneTransform);
    vec3 T = normalize(mat3(currentModelMatrix) * (boneTransformMat3 * inTangent));
    vec3 B = normalize(mat3(currentModelMatrix) * (boneTransformMat3 * inBitangent));
    vec3 N = normalize(mat3(currentModelMatrix) * (boneTransformMat3 * inNormal));
    fragTBN = mat3(T, B, N);
    fragNormal = N;

    // 텍스처 좌표는 그대로 전달
    fragTexCoord = inTexCoord;
}