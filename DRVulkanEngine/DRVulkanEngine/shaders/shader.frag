#version 450

// 입력(in) 변수 변경: location=0은 이제 다른 용도로 써야 하므로 1로 변경
// vec3 fragColor -> vec2 fragTexCoord
layout(location = 1) in vec2 fragTexCoord;

// 출력(out) 변수
layout(location = 0) out vec4 outColor;

// Uniform 변수 추가: C++에서 전달할 텍스처와 샘플러
// binding = 1은 DescriptorSetLayout에서 설정한 바인딩 번호와 일치해야 합니다.
layout(binding = 1) uniform sampler2D texSampler;

void main() {
    // texture() 함수를 사용해 fragTexCoord 위치의 텍스처 색상을 가져옵니다.
    outColor = texture(texSampler, fragTexCoord);
}