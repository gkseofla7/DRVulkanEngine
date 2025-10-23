#version 450

// 입력(in) 변수: Vertex 구조체와 일치해야 합니다.
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent; // 필요하다면 C++에서 계산해서 넘겨주세요.

// 출력(out) 변수: 프래그먼트 셰이더로 넘겨줄 데이터
layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out mat3 fragTBN; // Tangent, Bitangent, Normal 정보를 행렬 하나로 묶어서 전달

// Uniform 변수: UBO (이제 Model, View, Proj 모두 포함)
// 렌더링 루프에서 오브젝트마다 업데이트되는 정보
layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    // 월드 좌표계에서의 정점 위치 계산
    vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
    fragWorldPos = worldPos.xyz;

    // 최종 클립 공간 위치 계산
    gl_Position = ubo.proj * ubo.view * worldPos;

    // 텍스처 좌표는 그대로 전달
    fragTexCoord = inTexCoord;

    // 법선, 탄젠트, 바이탄젠트를 월드 공간으로 변환 (회전만 적용)
    // mat3(ubo.model)은 모델 행렬에서 스케일과 이동을 제외한 순수 회전 정보만 가져옵니다.
    vec3 T = normalize(mat3(ubo.model) * inTangent);
    vec3 B = normalize(mat3(ubo.model) * inBitangent);
    vec3 N = normalize(mat3(ubo.model) * inNormal);
    fragTBN = mat3(T, B, N); // TBN 행렬 생성

    // fragNormal은 디버깅이나 간단한 조명에 사용할 수 있도록 따로 전달
    fragNormal = N;
}