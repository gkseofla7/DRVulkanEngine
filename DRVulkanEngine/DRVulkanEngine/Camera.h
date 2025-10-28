#pragma once

// GLM 라이브러리 헤더 포함
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Vulkan의 깊이 범위(0.0 to 1.0)에 맞춤
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    // 생성자: 초기 위치, 바라볼 지점, 월드의 '위' 방향을 받음
    Camera(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp) {
        m_Position = position;
        m_WorldUp = worldUp;
        m_Front = glm::normalize(target - position);
        m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
        m_Up = glm::normalize(glm::cross(m_Right, m_Front));
    }

    // View Matrix를 반환하는 함수
    // 카메라의 위치와 방향을 기반으로 계산됩니다.
    glm::mat4 getViewMatrix() const {
        // glm::lookAt(카메라 위치, 바라볼 목표 지점, 위쪽 방향)
        return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
    }

    // Projection Matrix를 반환하는 함수
    // 원근감을 적용합니다.
    glm::mat4 getProjectionMatrix(float fov, float aspectRatio, float near, float far) const {
        // glm::perspective(시야각(FOV), 화면비, Near Plane, Far Plane)
        glm::mat4 proj = glm::perspective(glm::radians(fov), aspectRatio, near, far);

        // GLM은 OpenGL 기준(-1 to 1)이므로 Y 좌표를 뒤집어 Vulkan에 맞춤
        proj[1][1] *= -1;

        return proj;
    }

    glm::vec3 getPosition() { return m_Position; }
private:
    glm::vec3 m_Position; // 카메라의 월드 공간 위치
    glm::vec3 m_Front;    // 카메라가 바라보는 방향 벡터
    glm::vec3 m_Up;       // 카메라의 '위' 벡터
    glm::vec3 m_Right;    // 카메라의 '오른쪽' 벡터
    glm::vec3 m_WorldUp;  // 월드 좌표계의 '위' 벡터
};