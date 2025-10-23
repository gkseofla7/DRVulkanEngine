#version 450

// �Է�(in) ����: ���ؽ� ���̴��� ��°� ��Ȯ�� ��ġ�ؾ� �մϴ�.
layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in mat3 fragTBN;

// ���(out) ����
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

// ���� �� ī�޶� ������ ���� UBO (����)
// �� UBO�� Set 0 �Ǵ� ������ Set 2�� ��ġ�� �� �ֽ��ϴ�.
layout(set = 0, binding = 1) uniform SceneUBO {
    vec3 lightPos;
    vec3 viewPos;
} scene;

void main() {
    // --- 1. ����(Normal) ��� ---
    vec3 normal = normalize(fragNormal);
    if (material.useNormalTex == 1) {
        // ��ָʿ��� ���� ������ ���ø��ϰ� [-1, 1] ������ ��ȯ
        vec3 tangentNormal = texture(texNormal, fragTexCoord).xyz * 2.0 - 1.0;
        // TBN ����� �̿��� ź��Ʈ ������ ������ ���� �������� ��ȯ
        normal = normalize(fragTBN * tangentNormal);
    }

    // --- 2. ���� ��꿡 �ʿ��� ���͵� �غ� ---
    vec3 lightDir = normalize(scene.lightPos - fragWorldPos); // ���� ����
    vec3 viewDir = normalize(scene.viewPos - fragWorldPos);   // �ü� ����
    vec3 halfwayDir = normalize(lightDir + viewDir);           // ���� ���� (Blinn-Phong)

    // --- 3. ���� ��� ��� ---
    // �ֺ��� (Ambient): �⺻ ���� + AO��
    float ambientOcclusion = (material.useAmbientTex == 1) ? texture(texAmbient, fragTexCoord).r : 1.0;
    vec3 ambient = 0.1 * texture(texDiffuse, fragTexCoord).rgb * ambientOcclusion;

    // ���ݻ籤 (Diffuse)
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * texture(texDiffuse, fragTexCoord).rgb;

    // ���ݻ籤 (Specular)
    float specStrength = (material.useSpecularTex == 1) ? texture(texSpecular, fragTexCoord).r : 0.5;
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0); // 32.0 = shininess
    vec3 specular = specStrength * spec * vec3(1.0, 1.0, 1.0); // ���� ����

    // ���Ɽ (Emissive)
    vec3 emissive = (material.useEmissiveTex == 1) ? texture(texEmissive, fragTexCoord).rgb : vec3(0.0);

    // --- 4. ���� ���� ���� ---
    vec3 result = ambient + diffuse + specular + emissive;
    outColor = vec4(result, 1.0);
}