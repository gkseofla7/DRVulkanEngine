#include "Camera.h"
#include "Camera.h"

// 생성자
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : m_Position(position), m_WorldUp(up), m_Yaw(yaw), m_Pitch(pitch),
    m_Front(glm::vec3(0.0f, 0.0f, -1.0f)), m_MovementSpeed(5.0f), m_MouseSensitivity(0.1f), m_Fov(45.0f)
{
    updateCameraVectors();
}

// View Matrix 계산
glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

// Projection Matrix 계산
glm::mat4 Camera::getProjectionMatrix(float aspectRatio, float near, float far) const {
    glm::mat4 proj = glm::perspective(glm::radians(m_Fov), aspectRatio, near, far);
    proj[1][1] *= -1; // Vulkan을 위해 Y좌표 반전
    return proj;
}

// 키보드 입력 처리
void Camera::processKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = m_MovementSpeed * deltaTime;
    if (direction == Camera_Movement::FORWARD)
        m_Position += m_Front * velocity;
    if (direction == Camera_Movement::BACKWARD)
        m_Position -= m_Front * velocity;
    if (direction == Camera_Movement::LEFT)
        m_Position -= m_Right * velocity;
    if (direction == Camera_Movement::RIGHT)
        m_Position += m_Right * velocity;
    if (direction == Camera_Movement::UP)
        m_Position += m_WorldUp * velocity; // 월드 기준 위로 이동
    if (direction == Camera_Movement::DOWN)
        m_Position -= m_WorldUp * velocity; // 월드 기준 아래로 이동
}

// 마우스 움직임 처리
void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= m_MouseSensitivity;
    yoffset *= m_MouseSensitivity;

    m_Yaw += xoffset;
    m_Pitch += yoffset;

    // Pitch(상하 회전)가 90도를 넘어가지 않도록 제한 (화면 뒤집힘 방지)
    if (constrainPitch) {
        if (m_Pitch > 89.0f)
            m_Pitch = 89.0f;
        if (m_Pitch < -89.0f)
            m_Pitch = -89.0f;
    }

    // 변경된 Yaw와 Pitch를 바탕으로 방향 벡터들을 다시 계산
    updateCameraVectors();
}

// 마우스 스크롤(줌) 처리
void Camera::processMouseScroll(float yoffset) {
    m_Fov -= yoffset;
    if (m_Fov < 1.0f)
        m_Fov = 1.0f;
    if (m_Fov > 60.0f) // 최대 시야각은 60도로 제한
        m_Fov = 60.0f;
}


void Camera::updateCameraVectors() {
    // 새로운 Front 벡터 계산
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);

    // Right와 Up 벡터 다시 계산
    // 먼저 Right 벡터를 구한 뒤, 그 결과로 Up 벡터를 구해야 항상 서로 직교 관계가 유지됩니다.
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}