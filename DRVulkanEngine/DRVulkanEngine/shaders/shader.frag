#version 450

// �Է�(in) ���� ����: location=0�� ���� �ٸ� �뵵�� ��� �ϹǷ� 1�� ����
// vec3 fragColor -> vec2 fragTexCoord
layout(location = 1) in vec2 fragTexCoord;

// ���(out) ����
layout(location = 0) out vec4 outColor;

// Uniform ���� �߰�: C++���� ������ �ؽ�ó�� ���÷�
// binding = 1�� DescriptorSetLayout���� ������ ���ε� ��ȣ�� ��ġ�ؾ� �մϴ�.
layout(binding = 1) uniform sampler2D texSampler;

void main() {
    // texture() �Լ��� ����� fragTexCoord ��ġ�� �ؽ�ó ������ �����ɴϴ�.
    outColor = texture(texSampler, fragTexCoord);
}