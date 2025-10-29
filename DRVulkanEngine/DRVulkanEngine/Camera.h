#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// 카메라 이동 방향을 나타내는 열거형
enum class Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
public:
    // --- 생성자 ---
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = -90.0f, float pitch = 0.0f);

    // --- 행렬 반환 함수 ---
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspectRatio, float near, float far) const;

    // --- 입력 처리 함수 ---
    // 키보드 입력을 처리하여 위치를 변경합니다.
    void processKeyboard(Camera_Movement direction, float deltaTime);
    // 마우스 움직임을 처리하여 Yaw와 Pitch를 변경합니다.
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    // 마우스 스크롤을 처리하여 줌(FOV)을 변경합니다.
    void processMouseScroll(float yoffset);

    // --- Getter 함수 ---
    glm::vec3 getPosition() const { return m_Position; }
    glm::vec3 getFront() const { return m_Front; }

private:
    // 카메라의 방향 벡터들을 Yaw와 Pitch로부터 다시 계산합니다.
    void updateCameraVectors();

    // --- 카메라 속성 ---
    glm::vec3 m_Position;
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;

    // --- 오일러 각 (Euler Angles) ---
    float m_Yaw;
    float m_Pitch;

    // --- 카메라 옵션 ---
    float m_MovementSpeed;
    float m_MouseSensitivity;
    float m_Fov; // Field of View (시야각)
};