#version 450

// 입력(in) 변수: 버텍스 셰이더의 출력과 정확히 일치해야 합니다.
layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in mat3 fragTBN;

// 출력(out) 변수
layout(location = 0) out vec4 outColor;

// --- Set 1: Material UBO ---
layout(set = 1, binding = 0) uniform MaterialUBO {
    int useNormalTex;
    int useSpecularTex;
    int useAmbientTex;
    int useEmissiveTex;
} material;

layout(set = 2, binding = 0) uniform sampler2D texDiffuse;
layout(set = 2, binding = 1) uniform sampler2D texSpecular;
layout(set = 2, binding = 2) uniform sampler2D texNormal;
layout(set = 2, binding = 3) uniform sampler2D texAmbient;
layout(set = 2, binding = 4) uniform sampler2D texEmissive;

// 조명 및 카메라 정보를 담을 UBO (예시)
// 이 UBO는 Set 0 또는 별도의 Set 2에 배치할 수 있습니다.
layout(set = 0, binding = 1) uniform SceneUBO {
    vec3 lightPos;
    vec3 viewPos;
} scene;

void main() {
    // --- 1. 법선(Normal) 계산 ---
    vec3 normal = normalize(fragNormal);
    if (material.useNormalTex == 1) {
        // 노멀맵에서 법선 정보를 샘플링하고 [-1, 1] 범위로 변환
        vec3 tangentNormal = texture(texNormal, fragTexCoord).xyz * 2.0 - 1.0;
        // TBN 행렬을 이용해 탄젠트 공간의 법선을 월드 공간으로 변환
        normal = normalize(fragTBN * tangentNormal);
    }

    // --- 2. 조명 계산에 필요한 벡터들 준비 ---
    vec3 lightDir = normalize(scene.lightPos - fragWorldPos); // 광원 방향
    vec3 viewDir = normalize(scene.viewPos - fragWorldPos);   // 시선 방향
    vec3 halfwayDir = normalize(lightDir + viewDir);           // 하프 벡터 (Blinn-Phong)

    // --- 3. 조명 요소 계산 ---
    // 주변광 (Ambient): 기본 조명 + AO맵
    float ambientOcclusion = (material.useAmbientTex == 1) ? texture(texAmbient, fragTexCoord).r : 1.0;
    vec3 ambient = 0.1 * texture(texDiffuse, fragTexCoord).rgb * ambientOcclusion;

    // 난반사광 (Diffuse)
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * texture(texDiffuse, fragTexCoord).rgb;

    // 정반사광 (Specular)
    float specStrength = (material.useSpecularTex == 1) ? texture(texSpecular, fragTexCoord).r : 0.5;
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0); // 32.0 = shininess
    vec3 specular = specStrength * spec * vec3(1.0, 1.0, 1.0); // 광원 색상

    // 방출광 (Emissive)
    vec3 emissive = (material.useEmissiveTex == 1) ? texture(texEmissive, fragTexCoord).rgb : vec3(0.0);

    // --- 4. 최종 색상 조합 ---
    vec3 result = ambient + diffuse + specular + emissive;
    outColor = vec4(result, 1.0);
}