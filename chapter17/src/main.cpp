#include <iostream>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "camera.h"
#include "shader.h"

// 视口内容（颜色纹理附件）随窗口尺寸变化
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
// 鼠标指针事件
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
// 鼠标滚轮事件
void scroll_callback(GLFWwindow* window, double xoffsetIn, double yoffsetIn);
// 按键事件
void processInput(GLFWwindow *window);
// 按键事件工具函数：切换至全屏独占模式
void switch2Full(GLFWwindow *window);
// 按键事件工具函数：切换至窗口化模式
void switch2Win(GLFWwindow *window);

bool VSync = false;	// 垂直同步状态
bool isFullscreen = false;	// 是否处于全屏独占模式
int windowedPosX, windowedPosY, windowedWidth, windowedHeight;	// 保存窗口化模式相关数据

float delta = 0.0f;     // 帧间时差
float lastFrame = 0.0f; // 末帧时间
auto myCamera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, -1.5f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f);  
int screenWidth = 1000;
int screenHeight = 800;	// 设置显示宽高
float Camera::m_cursorX = screenWidth / 2.0f;
float Camera::m_cursorY = screenHeight / 2.0f;	// 鼠标指针位置窗口居中
bool Camera::firstMouse = true;
bool Camera::IsMouseCaptured = true;

int main() {
	// 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // 创建窗口
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Learning Chapter17", NULL, NULL);
    if (window == NULL) {
		std::cout << "Create GLFW Window Failed!" << std::endl;
        glfwTerminate();
        return -1;
    }
    // 设置上下文
    glfwMakeContextCurrent(window);
    // 窗口初始化
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Initial GLFW Windows Failed!" << std::endl;
        return -1;
    }
    // 视口尺寸
    glViewport(0, 0, screenWidth, screenHeight);
	// 同步窗口变化视口尺寸
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	// 默认关闭垂直同步
	glfwSwapInterval(VSync);
	// 启用深度测试
	glEnable(GL_DEPTH_TEST);
	// 启用更改顶点大小选项
	glEnable(GL_PROGRAM_POINT_SIZE);
	// 检测鼠标移动事件并调用回调函数
	glfwSetCursorPosCallback(window, mouse_callback);
	// 检测键盘输入时间并调用回调函数
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// 检测鼠标滚轮事件并调用回调函数
	glfwSetScrollCallback(window, scroll_callback);

	// 元素缓冲对象
    float vertPos[] = {	// 坐标
		0.5f, -0.5f, 0.0f,
		0.5f,  0.5f, 0.0f,
	   -0.5f,  0.5f, 0.0f,
	   -0.5f, -0.5f, 0.0f
	};
    float vertCol[] = {	// 颜色
		0.99f, 0.55f, 0.23f, 
		0.99f, 0.55f, 0.23f, 
		0.99f, 0.55f, 0.23f, 
		0.99f, 0.55f, 0.23f
	};
    float vertNor[] = {	// 法线
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f
	};
    unsigned int indices[] = {
        0, 1, 2, 
        2, 3, 0
    };    
	// 生成配置 VAO、EBO、VBO
	unsigned int VAO, EBO, VBO;
    glGenVertexArrays(1, &VAO);   
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertPos) + sizeof(vertCol) + sizeof(vertNor), NULL, GL_STATIC_DRAW);	// 申请预留缓冲空间
	
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertPos), vertPos);	
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertPos), sizeof(vertCol), vertCol);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertPos) + sizeof(vertCol), sizeof(vertNor), vertNor);
 	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
    glEnableVertexAttribArray(0);	// 位置
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vertPos)));
    glEnableVertexAttribArray(1);	// 颜色
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vertPos) + sizeof(vertCol)));
    glEnableVertexAttribArray(2);	// 法线向量
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
	
	auto myShader = std::make_unique<Shader>("src/shaders/shader.vert", "src/shaders/shader.frag");
	// 设置相应绑定点
	unsigned int matrices_index = glGetUniformBlockIndex(myShader->ID, "Matrices");
	glUniformBlockBinding(myShader->ID, matrices_index, 0);
	// 设置 uniform 数据缓冲对象
	unsigned int UBO;
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	// 绑定 UBO 至绑定点
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	// 设置光源
	unsigned int lights_index = glGetUniformBlockIndex(myShader->ID, "Light");
	glUniformBlockBinding(myShader->ID, lights_index, 1);
	unsigned int uboLight;
	glGenBuffers(1, &uboLight);
	glBindBuffer(GL_UNIFORM_BUFFER, uboLight);
	GLsizeiptr uboLightSize = 5 * sizeof(float) + 5 * (sizeof(glm::vec3) + 4) + 12;
	glBufferData(GL_UNIFORM_BUFFER, uboLightSize, NULL, GL_STATIC_DRAW);	
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboLight);
	void* uboLight_ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, uboLightSize, GL_MAP_WRITE_BIT);
	if (uboLight_ptr) {
		// 传入光源 float 类型参数
		float* floatData = static_cast<float*>(uboLight_ptr);
		float innerCutOff{cosf(glm::radians(11.0f))}, outerCutOff{cosf(glm::radians(13.0f))};	// 设置聚光内外切光角余弦值	
		floatData[0] = innerCutOff;
		floatData[1] = outerCutOff;
		float constant{1.0f}, linear{0.2f}, quadratic{0.022f};	// 设置光照衰减函数常数项、一次项、二次项
		floatData[2] = constant;
		floatData[3] = linear;
		floatData[4] = quadratic;
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// 设置实例坐标偏移量
	unsigned int offsets_index = glGetUniformBlockIndex(myShader->ID, "InstanceOffsets");
	glUniformBlockBinding(myShader->ID, offsets_index, 2);
	unsigned int uboOffsets;
	glGenBuffers(1, &uboOffsets);
	glBindBuffer(GL_UNIFORM_BUFFER, uboOffsets);
	glBufferData(GL_UNIFORM_BUFFER, 125 * (sizeof(glm::vec3) + 4), NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboOffsets);
	void* uboOffsets_ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, 125 * (sizeof(glm::vec3) + 4), GL_MAP_WRITE_BIT);
	if (uboOffsets_ptr) {
		float spacing = 1.5f;	// 偏移步长
		glm::vec3 start = glm::vec3(-3.0f, -3.0f, 0.5f);	// 初始偏移量
		float* offsetsData = static_cast<float*>(uboOffsets_ptr);
		int idx = 0;
		for (int i = 0; i < 5; i++) {	// 迭代设置5*5范围内实例坐标
			float x = start.x + i * spacing;
			for (int j = 0; j < 5; j++) {
				float y = start.y + j * spacing;
				for (int k = 0; k < 5; k++) {
					float z = start.z + k * spacing;
					offsetsData[idx * 4] = x;
					offsetsData[idx * 4 + 1] = y;
					offsetsData[idx * 4 + 2] = z;
					offsetsData[idx * 4 + 3] = 0;	// 填充位
					idx++;
				}
			}
		}
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	while (!glfwWindowShouldClose(window)){
		processInput(window);
		
		// 背景重绘
        glClearColor(0.2f, 0.2f, 0.2f, 0.2f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
        // 传入光源 vec3 类型参数
		glBindBuffer(GL_UNIFORM_BUFFER, uboLight);
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);	    		// 设置光源颜色
        glm::vec3 direction =myCamera->m_front;     		// 设置光照方向同摄像机所摄方向
        glm::vec3 lightPos = myCamera->m_position;			// 设置光源位置同摄像机位置
        glm::vec3 diffuse = glm::vec3(0.5f) * lightColor;	// 设置漫反射光强分量
        glm::vec3 specular = glm::vec3(0.3f) * lightColor;	// 设置镜面反射光强分量
        glm::vec3 ambient = glm::vec3(0.4f) * lightColor;	// 设置环境光强分量
		void* uboLight_ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, uboLightSize, GL_MAP_WRITE_BIT);
		if (uboLight_ptr) {
			char* bytePtr = static_cast<char*>(uboLight_ptr);
			// 填充12字节偏移量对齐后再写入
			memcpy(bytePtr + 							   5 * sizeof(float) + 12, glm::value_ptr(lightPos),  sizeof(glm::vec3));
			memcpy(bytePtr + 	 (sizeof(glm::vec3) + 4) + 5 * sizeof(float) + 12, glm::value_ptr(direction), sizeof(glm::vec3));
			memcpy(bytePtr + 2 * (sizeof(glm::vec3) + 4) + 5 * sizeof(float) + 12, glm::value_ptr(diffuse),   sizeof(glm::vec3));
			memcpy(bytePtr + 3 * (sizeof(glm::vec3) + 4) + 5 * sizeof(float) + 12, glm::value_ptr(specular),  sizeof(glm::vec3));
			memcpy(bytePtr + 4 * (sizeof(glm::vec3) + 4) + 5 * sizeof(float) + 12, glm::value_ptr(ambient),   sizeof(glm::vec3));
			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glm::vec2 screenSize = glm::vec2(screenWidth, screenHeight);
		myShader->use();	
		myShader->setVec2("screenSize", screenSize);
		
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);	// 进入 UBO 状态机
		// 透视裁剪
        glm::mat4 projection = glm::perspective(glm::radians(myCamera->m_zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));	// UBO前半部分 写入透视矩阵
		// 观察视角
        glm::mat4 view = myCamera->GetViewMatrix();
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));	// UBO后半部分 写入视角矩阵
		glBindBuffer(GL_UNIFORM_BUFFER, 0);		// 退出 UBO 状态机
		// 位移缩放
		myShader->use();	
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        myShader->setMat4("model", model);
		
		// 绘制物体
		myShader->use();
		glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 125);	//使用实例化绘制125个正方形

		glfwSwapBuffers(window);
        glfwPollEvents();
	}
	// 释放资源
	glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
	myShader.reset();
    glfwTerminate();
	myCamera.reset();
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) { // 传入窗口句柄与尺寸（回调函数）
	if (width == 0 || height == 0) return;	
	glViewport(0, 0, width, height);
    screenWidth = width;
    screenHeight = height;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) { // 传入窗口句柄与鼠标坐标（回调函数）
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    myCamera->ProcessMouseMovement(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffsetIn, double yoffsetIn) {  // 传入窗口句柄与滚轮运动参数（回调函数）
    float yoffset = static_cast<float>(yoffsetIn);
    myCamera->ProcessMouseScroll(yoffset);
}

void processInput(GLFWwindow *window) { // 传入窗口句柄，实时监测按键事件
	// 按 ~ 切换鼠标捕获模式
    static bool lastGraveState = false;
    bool currentGraveState = glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS;
	if (currentGraveState && !lastGraveState) {
        int cursorMode = glfwGetInputMode(window, GLFW_CURSOR);
		if (cursorMode == GLFW_CURSOR_DISABLED) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			Camera::IsMouseCaptured = false;
        } 
		else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            Camera::firstMouse = true;
			Camera::IsMouseCaptured = true;
        }
		// 鼠标指针坐标归中
		Camera::m_cursorX = screenWidth / 2.0f;
		Camera::m_cursorY = screenHeight / 2.0f;
		glfwSetCursorPos(window, screenWidth / 2.0f, screenHeight / 2.0f);

		std::cout << "Cursor mode has been changed." << std::endl;
	}
	lastGraveState = currentGraveState;
	
	if (Camera::IsMouseCaptured) {
		// 按 F11 切换全屏独占模式
		static bool lastF11State = false;
		bool currentF11State = glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS;
		if (currentF11State && !lastF11State) {
			if (!isFullscreen) {
				switch2Full(window);	
				isFullscreen = true;
				std::cout << "Switched to fullscreen exclusive mode." << std::endl;
			} 
			else {
				switch2Win(window);		
				isFullscreen = false;
				std::cout << "Switched back to windowed mode." << std::endl;
			}
		}
		lastF11State = currentF11State;

		// 按下 WASD↑↓ 改变摄像机位置
		float currentFrame = glfwGetTime();
		delta = currentFrame - lastFrame;
		lastFrame = currentFrame;
		Camera::Camera_Movement direction = Camera::Camera_Movement::NOTMOVE;   // 默认不移动
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {     // W 前进
			direction = Camera::Camera_Movement::FORWARD;
		}    
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {     // S 后退
			direction = Camera::Camera_Movement::BACKWARD;
		}    
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {     // A 左移
			direction = Camera::Camera_Movement::LEFT;
		}    
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {     // D 右移
			direction = Camera::Camera_Movement::RIGHT;
		}
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {    // ↑ 上升
			direction = Camera::Camera_Movement::UP;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {  // ↓ 下降
			direction = Camera::Camera_Movement::DOWN;
		}
	
		// 按 R 恢复 fov
		static bool lastRState = false;
		bool currentRState = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
		if (currentRState && !lastRState) {
			direction = Camera::REZOOM;
			std::cout << "The FOV has been reset to default value." << std::endl;
		}
		lastRState = currentRState;
		myCamera->ProcessKeyboard(direction, delta);

		// 按 V 控制垂直同步开闭
		static bool lastVState = false;
		bool currentVState = glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS;
		if (currentVState && !lastVState) {
			if (VSync) {
				VSync = false;
				if (isFullscreen) {
					glfwSwapInterval(VSync);
					std::cout << "The VSync has been disabled." << std::endl;
				}
				else {	// 窗口化模式下必须进入全屏独占模式才可脱离硬件调度强制更改垂直缓冲模式
					switch2Full(window);
					glfwSwapBuffers(window);	// 交换缓冲刷新状态
					glfwSwapInterval(VSync);
					switch2Win(window);
				}	
				std::cout << "The VSync has been disabled." << std::endl;
			}
			else {
				VSync = true;
				if (isFullscreen) {
					glfwSwapInterval(VSync);
				}
				else {
					switch2Full(window);
					glfwSwapBuffers(window);
					glfwSwapInterval(VSync);
					switch2Win(window);
				}	
				std::cout << "The VSync has been enabled." << std::endl;
			}
		}
		lastVState = currentVState;
	}

    // 按下 Esc 退出窗口
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void switch2Full(GLFWwindow *window){
	// 保存初始窗口化状态
	glfwGetWindowPos(window, &windowedPosX, &windowedPosY);
	glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
	// 切换到全屏独占
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();	// 返回用户系统的主显示器
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);	// 获取显示器视频模式，数据包括分辨率宽高、RGB色位深、刷新率
	glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);	// 渲染画面切换至主显示器，即全屏独占显示
	// 重置光标位置
	Camera::firstMouse = true;
	glfwSetCursorPos(window, screenWidth / 2.0f, screenHeight / 2.0f);

}

void switch2Win(GLFWwindow *window){
	// 恢复窗口化模式
	glfwSetWindowMonitor(window, NULL, windowedPosX, windowedPosY, windowedWidth, windowedHeight, 0);
	// 重置光标位置
	Camera::firstMouse = true;
	glfwSetCursorPos(window, screenWidth / 2.0f, screenHeight / 2.0f);
}