#include "camera.h"

// 初始化列表：摄像机所摄方向，摄像机移动速度，鼠标灵敏度，fov值，摄像机高度
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) 
: m_front(glm::vec3(0.0f, 0.0f, -1.0f)), m_movementSpeed(SPEED), m_mouseSensitivity(SENSITIVITY), m_zoom(ZOOM), m_posY(HEIGHT){
    m_position = position;
    m_worldUp = up;
    m_yaw = yaw;
    m_pitch = pitch;
    updateCameraVectors();
}
Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) 
: m_front(glm::vec3(0.0f, 0.0f, -1.0f)), m_movementSpeed(SPEED), m_mouseSensitivity(SENSITIVITY), m_zoom(ZOOM), m_posY(HEIGHT){
    m_position = glm::vec3(posX, posY, posZ);
    m_worldUp = glm::vec3(upX, upY, upZ);
    m_yaw = yaw;
    m_pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() {
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::lookAt(m_position, m_position + m_front, m_up);
    return view;
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
	float velocity = m_movementSpeed * deltaTime;
	if (!m_fly) {m_posY = m_position.y;}
	switch(direction) {
		case Camera_Movement::NOTMOVE: 
		{if (!m_fly) {m_position.y = m_posY;}}
		return;
		case Camera_Movement::FORWARD:
			{m_position += m_front * velocity;}
		break;
		case Camera_Movement::BACKWARD:
			{m_position -= m_front * velocity;}
		break;
		case Camera_Movement::LEFT:
			{m_position -= m_right * velocity;}
		break;
		case Camera_Movement::RIGHT:
			{m_position += m_right * velocity;}
		break;
		case Camera_Movement::REZOOM:
			{m_zoom = ZOOM;}
		break;
		default:
		break;
	}
	if (!m_fly) {
		m_position.y = m_posY;	// 禁止飞行
	}
}

void Camera::ProcessMouseMovement(float cursorX, float cursorY, GLboolean constrainPitch) {
	if (firstMouse) {
		m_cursorX = cursorX;
		m_cursorY = cursorY;
		firstMouse = false;
    }
	float xoffset = cursorX - m_cursorX;
	float yoffset = m_cursorY - cursorY;
	m_cursorX = cursorX;
	m_cursorY = cursorY;
	xoffset *= m_mouseSensitivity;
	yoffset *= m_mouseSensitivity;
	m_yaw   += xoffset;
	m_pitch += yoffset;

	if (constrainPitch) {
		if (m_pitch > 89.9f)
			m_pitch = 89.9f;
		if (m_pitch < -89.9f)
			m_pitch = -89.9f;
	}
	updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
	m_zoom -= (float)yoffset;
	if (m_zoom < 1.0f)
		m_zoom = 1.0f;
	if (m_zoom > 120.0f)
		m_zoom = 120.0f;
}

void Camera::updateCameraVectors() {
	// 计算新所摄方向向量
	glm::vec3 front;
	front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	front.y = sin(glm::radians(m_pitch));
	front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_front = glm::normalize(front);
	// 计算新正x轴与正y轴方向向量
	m_right = glm::normalize(glm::cross(m_front, m_worldUp));
	m_up    = glm::normalize(glm::cross(m_right, m_front));
}
