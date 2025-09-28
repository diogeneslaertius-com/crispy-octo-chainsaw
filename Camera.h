#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
	Camera(glm::vec3 position);

	glm::mat4 ProcessInput(GLFWwindow* window, float deltaTime);
	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix(GLFWwindow* window, float fov, float nearPlane, float farPlane) const;
	glm::vec3 GetPosition() const { return m_Position; }
	glm::vec3 GetFront() const { return m_Front; }

	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
	void SetPosition(const glm::vec3& pos) { m_Position = pos; }
	float m_LastX = 400.0f;
	float m_LastY = 300.0f; // Начальная позиция Y (центр экрана)
	bool m_FirstMouse = true; //

private:
	glm::vec3 m_Position = glm::vec3(0.0f);
	glm::vec3 m_Front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
	float m_Yaw = -90.0f;
	float m_Pitch = 0.0f;
	float m_MouseSensitivity = 0.1f; // Чувствительность

	void updateCameraVectors();
};