#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
layout(location = 5) in ivec4 inBoneIDs;
layout(location = 6) in vec4 inWeights;

layout (location = 0) out vec3 outTexCoord;

layout(set = 0, binding = 0) uniform SceneUBO {
    mat4 proj;
    mat4 view;
    vec3 lightPos;
    vec3 viewPos;
} scene;

void main() {
    outTexCoord = inPosition;
    outTexCoord.y = -outTexCoord.y;

    mat4 viewNoTranslation = mat4(mat3(scene.view));

    vec4 pos = scene.proj * viewNoTranslation * vec4(inPosition, 1.0);

    gl_Position = pos.xyww;
}