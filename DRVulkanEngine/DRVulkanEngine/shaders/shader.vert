#version 450

// �Է�(in) ����: ���� �����Ϳ��� ��ġ, ����, �ؽ�ó ��ǥ�� �޽��ϴ�.
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord; // �ؽ�ó ��ǥ �߰�!

// ���(out) ����: �����׸�Ʈ ���̴��� �Ѱ��� ������
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord; // �ؽ�ó ��ǥ �߰�!

// Uniform ����: UBO
layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * vec4(inPosition, 1.0);
    
    // �Է¹��� �����͸� �����׸�Ʈ ���̴��� �״�� �����մϴ�.
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}