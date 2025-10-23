#version 450

// �Է�(in) ����: Vertex ����ü�� ��ġ�ؾ� �մϴ�.
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent; // �ʿ��ϴٸ� C++���� ����ؼ� �Ѱ��ּ���.

// ���(out) ����: �����׸�Ʈ ���̴��� �Ѱ��� ������
layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out mat3 fragTBN; // Tangent, Bitangent, Normal ������ ��� �ϳ��� ��� ����

// Uniform ����: UBO (���� Model, View, Proj ��� ����)
// ������ �������� ������Ʈ���� ������Ʈ�Ǵ� ����
layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    // ���� ��ǥ�迡���� ���� ��ġ ���
    vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
    fragWorldPos = worldPos.xyz;

    // ���� Ŭ�� ���� ��ġ ���
    gl_Position = ubo.proj * ubo.view * worldPos;

    // �ؽ�ó ��ǥ�� �״�� ����
    fragTexCoord = inTexCoord;

    // ����, ź��Ʈ, ����ź��Ʈ�� ���� �������� ��ȯ (ȸ���� ����)
    // mat3(ubo.model)�� �� ��Ŀ��� �����ϰ� �̵��� ������ ���� ȸ�� ������ �����ɴϴ�.
    vec3 T = normalize(mat3(ubo.model) * inTangent);
    vec3 B = normalize(mat3(ubo.model) * inBitangent);
    vec3 N = normalize(mat3(ubo.model) * inNormal);
    fragTBN = mat3(T, B, N); // TBN ��� ����

    // fragNormal�� ������̳� ������ ���� ����� �� �ֵ��� ���� ����
    fragNormal = N;
}