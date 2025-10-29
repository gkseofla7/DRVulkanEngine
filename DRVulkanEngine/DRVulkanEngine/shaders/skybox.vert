#version 450

// 입력: 큐브의 로컬 정점 위치
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
layout(location = 5) in ivec4 inBoneIDs;
layout(location = 6) in vec4 inWeights;

// 출력: 프래그먼트 셰이더로 전달할 3D 텍스처 좌표 (월드 공간 방향 벡터)
layout (location = 0) out vec3 outTexCoord;

layout(set = 0, binding = 0) uniform SceneUBO {
    mat4 proj;
    mat4 view;
    vec3 lightPos;
    vec3 viewPos;
} scene;

void main() {
    // 텍스처 좌표로 정점 위치를 그대로 사용합니다.
    // 이 벡터가 큐브맵을 샘플링할 방향이 됩니다.
    outTexCoord = inPosition;
    outTexCoord.y = -outTexCoord.y;

    // 뷰 행렬에서 이동(Translation) 부분을 제거합니다.
    // mat3으로 변환하여 회전 정보만 남기고, 다시 mat4로 만듭니다.
    // 이렇게 하면 스카이박스가 카메라의 움직임(이동)에 영향을 받지 않고 회전만 따라갑니다.
    mat4 viewNoTranslation = mat4(mat3(scene.view));

    // 최종 클립 공간 위치 계산
    vec4 pos = scene.proj * viewNoTranslation * vec4(inPosition, 1.0);

    // 깊이 값을 항상 최댓값(1.0)으로 설정합니다.
    // gl_Position.w는 깊이 버퍼에서 z값을 나누는 데 사용됩니다.
    // z를 w와 같게 만들면, 투영 나누기(z/w) 이후의 최종 깊이(NDC z)는 1.0이 됩니다.
    // 이렇게 하면 스카이박스가 씬의 가장 뒤에 그려지게 됩니다.
    gl_Position = pos.xyww;
}