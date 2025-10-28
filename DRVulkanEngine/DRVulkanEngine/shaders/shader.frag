#version 450

#define MAX_MATERIALS 128
#define MAX_TEXTURES_PER_MATERIAL 128

// --- 입력(in) 변수 ---
layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in mat3 fragTBN;

// --- 출력(out) 변수 ---
layout(location = 0) out vec4 outColor;

// --- PushConstant ---
layout(push_constant) uniform PushConstants {
    int modelUBIndex;  // offset = 0
    int materialIndex; // offset = 4
} pc;
// --- DescriptorSet ---
// Set 0: 씬 데이터
layout(set = 0, binding = 1) uniform SceneUBO {
    vec3 lightPos;
    vec3 viewPos;
} scene;

// Set 1: 머티리얼 데이터 배열
struct MaterialUBO {
    int diffuseTexIndex;
    int normalTexIndex;
    int specularTexIndex;
    int ambientTexIndex;
    int emissiveTexIndex;
};

layout(std140, set = 1, binding = 0) uniform MaterialCollection {
    MaterialUBO materials;
} materialData[MAX_MATERIALS];

// Set 2: 개별 텍스처(sampler2D)의 배열
layout(set = 2, binding = 0) uniform sampler2D materialTextures[MAX_TEXTURES_PER_MATERIAL];


void main() {
    MaterialUBO currentMaterial = materialData[pc.materialIndex].materials;

    vec3 normal = normalize(fragNormal);
    if (currentMaterial.normalTexIndex > -1) {
        vec3 tangentNormal = texture(materialTextures[currentMaterial.normalTexIndex], fragTexCoord).xyz * 2.0 - 1.0;
        normal = normalize(fragTBN * tangentNormal);
    }

    vec3 lightDir = normalize(scene.lightPos - fragWorldPos);
    vec3 viewDir = normalize(scene.viewPos - fragWorldPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // 주변광 (Ambient)
    float ambientOcclusion = 1.0;
    if (currentMaterial.ambientTexIndex > -1) {
        ambientOcclusion = texture(materialTextures[currentMaterial.ambientTexIndex], fragTexCoord).r;
    }
    vec3 ambientColor = texture(materialTextures[currentMaterial.diffuseTexIndex], fragTexCoord).rgb;
    vec3 ambient = 0.1 * ambientColor * ambientOcclusion;

    // 난반사광 (Diffuse)
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuseColor = texture(materialTextures[currentMaterial.diffuseTexIndex], fragTexCoord).rgb;
    vec3 diffuse = diff * diffuseColor;

    // 정반사광 (Specular)
    float specStrength = 0.5;
    if (currentMaterial.specularTexIndex > -1) {
        specStrength = texture(materialTextures[currentMaterial.specularTexIndex], fragTexCoord).r;
    }
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = specStrength * spec * vec3(1.0);

    // 방출광 (Emissive)
    vec3 emissive = vec3(0.0);
    if (currentMaterial.emissiveTexIndex > -1) {
        emissive = texture(materialTextures[currentMaterial.emissiveTexIndex], fragTexCoord).rgb;
    }

    vec3 result = ambient + diffuse + specular + emissive;
    outColor = vec4(result, 1.0);
}