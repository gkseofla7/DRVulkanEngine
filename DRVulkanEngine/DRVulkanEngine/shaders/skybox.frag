#version 450

// 입력: 버텍스 셰이더에서 보간된 텍스처 좌표 (방향 벡터)
layout (location = 0) in vec3 inTexCoord;

// 출력: 최종 픽셀 색상
layout (location = 0) out vec4 outColor;

// HDR 큐브맵 텍스처 샘플러
layout (set = 0, binding = 1) uniform samplerCube skyboxSampler;

void main() {
    // 정규화된 방향 벡터를 사용하여 큐브맵을 샘플링합니다.
    // HDR 큐브맵이므로 결과 색상 값은 1.0을 초과할 수 있습니다.
    // 이 값은 나중에 톤 매핑(Tone Mapping) 패스에서 처리됩니다.
    outColor = texture(skyboxSampler, inTexCoord);
}