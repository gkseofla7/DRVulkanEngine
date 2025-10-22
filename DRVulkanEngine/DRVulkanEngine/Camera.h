#pragma once

// GLM ���̺귯�� ��� ����
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Vulkan�� ���� ����(0.0 to 1.0)�� ����
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    // ������: �ʱ� ��ġ, �ٶ� ����, ������ '��' ������ ����
    Camera(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp) {
        m_Position = position;
        m_WorldUp = worldUp;
        m_Front = glm::normalize(target - position);
        m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
        m_Up = glm::normalize(glm::cross(m_Right, m_Front));
    }

    // View Matrix�� ��ȯ�ϴ� �Լ�
    // ī�޶��� ��ġ�� ������ ������� ���˴ϴ�.
    glm::mat4 getViewMatrix() const {
        // glm::lookAt(ī�޶� ��ġ, �ٶ� ��ǥ ����, ���� ����)
        return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
    }

    // Projection Matrix�� ��ȯ�ϴ� �Լ�
    // ���ٰ��� �����մϴ�.
    glm::mat4 getProjectionMatrix(float fov, float aspectRatio, float near, float far) const {
        // glm::perspective(�þ߰�(FOV), ȭ���, Near Plane, Far Plane)
        glm::mat4 proj = glm::perspective(glm::radians(fov), aspectRatio, near, far);

        // GLM�� OpenGL ����(-1 to 1)�̹Ƿ� Y ��ǥ�� ������ Vulkan�� ����
        proj[1][1] *= -1;

        return proj;
    }
private:
    glm::vec3 m_Position; // ī�޶��� ���� ���� ��ġ
    glm::vec3 m_Front;    // ī�޶� �ٶ󺸴� ���� ����
    glm::vec3 m_Up;       // ī�޶��� '��' ����
    glm::vec3 m_Right;    // ī�޶��� '������' ����
    glm::vec3 m_WorldUp;  // ���� ��ǥ���� '��' ����
};