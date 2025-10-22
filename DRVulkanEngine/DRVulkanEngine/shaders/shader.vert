#version 450

// 입력(in) 변수: 정점 데이터에서 위치, 색상, 텍스처 좌표를 받습니다.
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord; // 텍스처 좌표 추가!

// 출력(out) 변수: 프래그먼트 셰이더로 넘겨줄 데이터
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord; // 텍스처 좌표 추가!

// Uniform 변수: UBO
layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * vec4(inPosition, 1.0);
    
    // 입력받은 데이터를 프래그먼트 셰이더로 그대로 전달합니다.
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}