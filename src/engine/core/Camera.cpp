#include "Camera.hpp"

#include <GLFW/glfw3.h>

namespace lei3d
{
	Camera::Camera(GLFWwindow* window, float yaw, float pitch, LookMode mode)
		: m_Yaw{ yaw }, m_Pitch{ pitch }, m_LookMode{ mode }
	{
		m_FOVDeg = 45.0f;
		m_NearPlane = 0.1f;
		m_FarPlane = 1400.0f;

		int screenWidth, screenHeight;
		glfwGetWindowSize(window, &screenWidth, &screenHeight);
		m_Aspect = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);

		m_CameraPos = glm::vec3(0.0f, 0.0f, 3.0f); // Why is this hard-coded as a 3?
		m_CameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
		m_CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		m_CameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
	}

	Camera::~Camera()
	{
		//: Clown Emoji
	}

	void Camera::cameraMouseCallback(double xPosInput, double yPosInput)
	{
		switch (m_LookMode)
		{
			case LookMode::FREE:
				FreeCameraControls(static_cast<float>(xPosInput), static_cast<float>(yPosInput));
				break;
			case LookMode::FIXED:
			default:
				break;
		}
	}

	void Camera::FreeCameraControls(float xpos, float ypos)
	{
		if (m_MouseEnterFlag)
		{
			m_PrevX = xpos;
			m_PrevY = ypos;
			m_MouseEnterFlag = false;
		}

		float xoffset = xpos - m_PrevX;
		float yoffset = m_PrevY - ypos; // openGL inverted y
		m_PrevX = xpos;
		m_PrevY = ypos;

		xoffset *= MOUSE_SENSITIVITY;
		yoffset *= MOUSE_SENSITIVITY;

		m_Yaw += xoffset;
		m_Pitch += yoffset;

		if (m_Pitch > MAX_PITCH)
			m_Pitch = MAX_PITCH;
		if (m_Pitch < -MAX_PITCH)
			m_Pitch = -MAX_PITCH;

		glm::vec3 direction;
		direction.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		direction.y = sin(glm::radians(m_Pitch));
		direction.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		m_CameraFront = glm::normalize(direction);
		m_CameraRight = glm::normalize(glm::cross(m_CameraFront, m_CameraUp));
	}

	glm::mat4 Camera::GetView()
	{
		return glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
	}

	glm::mat4 Camera::GetProj()
	{
		return glm::perspective(glm::radians(m_FOVDeg), m_Aspect, m_NearPlane, m_FarPlane);
	}

	glm::vec3 Camera::GetPosition() const
	{
		return m_CameraPos;
	}

	glm::vec3 Camera::GetFront() const
	{
		return m_CameraFront;
	}

	glm::vec3 Camera::GetUp() const
	{
		return m_CameraUp;
	}

	float Camera::GetFOV() const
	{
		return m_FOVDeg;
	}

	float Camera::GetYaw() const
	{
		return m_Yaw;
	}

	float Camera::GetPitch() const
	{
		return m_Pitch;
	}

	void Camera::SetFOV(float fovDeg)
	{
		m_FOVDeg = fovDeg;
	}

	void Camera::SetClipPlanes(float nearClip, float farClip)
	{
		m_NearPlane = nearClip;
		m_FarPlane = farClip;
	}

	void Camera::SetPosition(glm::vec3 pos)
	{
		m_CameraPos = pos;
	}
} // namespace lei3d