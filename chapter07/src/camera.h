#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const float YAW         = -90.0f;   // 默认偏航角
const float PITCH       =  0.0f;    // 默认俯仰角
const float SPEED       =  2.5f;    // 默认速度
const float SENSITIVITY =  0.03f;   // 默认灵敏度
const float ZOOM        =  45.0f;   // 默认FOV缩放大小

class Camera {
public:
	enum KeyMouse_Control {		// 键鼠控制标志
		ORBITING,		// 摄像机环绕拍摄
		MOVING,			// 摄像机自由移动
		CATCH_CURSOR,	// 捕获鼠标
		RELEASE_CURSOR	// 释放鼠标
	};

	enum Camera_Movement {		// 摄像机移动标志
		FORWARD,    // W
		BACKWARD,   // S
		LEFT,       // A
		RIGHT,      // D
		NONE		// 不移动
	};

	static bool lastKeyState;

	glm::vec3 m_position; // 摄像机全局坐标
	glm::vec3 m_front;    // 摄像机所摄方向（负z轴方向）
	glm::vec3 m_up;       // 摄像机正y轴方向
	glm::vec3 m_right;    // 摄像机正x轴方向
	glm::vec3 m_worldUp;  // 全局正y轴方向

	int m_cursor = CATCH_CURSOR;	// 默认捕获鼠标
	int m_motion = ORBITING; 	// 默认环绕拍摄

    float m_yaw;			// 偏航角
    float m_pitch;  		// 俯仰角  

    float m_movementSpeed;		// 摄像机移动速度
    float m_mouseSensitivity;	// 鼠标灵敏度

	bool firstMouse = true;
    static float m_zoom;	// FOV缩放大小
	static float m_cursorX;	// 鼠标指针x坐标
	static float m_cursorY;	// 鼠标指针y坐标

	// 构造函数（摄像机全局坐标，全局正y轴方向，偏航角，俯仰角）
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
    // 构造函数（摄像机全局坐标x,y,z, 全局正y轴方向向量x,y,z， 偏航角，俯仰角）
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw = YAW, float pitch = PITCH);

    // 返回 LookAt 矩阵
    glm::mat4 GetViewMatrix();

	// 接受窗口指针与键盘事件布尔标志参数实现摄像头行为转变
	void SwitchBehaviour(GLFWwindow *m_window, bool currentKeyState);
    // 接受摄像机移动标志与帧间时差参数实现键盘控制
    void ProcessKeyboard( Camera_Movement direction, float deltaTime);

	// 接受鼠标移动的x轴分量、y轴分量与是否限制仰角参数实现鼠标控制
    void ProcessMouseMovement(float cursorX, float cursorY, GLboolean constrainPitch = true);
	
    // 接受鼠标滚轮水平滚动偏移量与垂直滚动偏移量参数实现FOV控制
    float ProcessMouseScroll(float yoffset);

private:
    // 更新摄像机所摄方向向量（负z轴）
    void updateCameraVectors();
};

#endif