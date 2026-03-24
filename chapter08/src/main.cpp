#include <iostream>
#include <cmath>
#include <iomanip>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"

int screenWidth = 800;
int screenHeight = 800;	// 设置显示宽高
float deltaTime = 0.0f; // 帧间时差
float lastFrame = 0.0f; // 上一帧时间点

double lastTime = glfwGetTime();    // 当前帧时间
int frameCount = 0;                 // 帧计数
float fps, lastFps = 0.0f;          // 前一刻fps与当前fps

Camera myCamera = Camera(glm::vec3(0.0f, 1.0f, 2.0f));  // 使用自建的摄像机类
float Camera::m_cursorX = screenWidth / 2.0f;
float Camera::m_cursorY = screenHeight / 2.0f;
bool Camera::firstMouse = true;

// 窗口随视口尺寸变化
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
// 获取实时帧率
float updateFPS();
// 鼠标指针事件
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
// 鼠标滚轮事件
void scroll_callback(GLFWwindow* window, double xoffsetIn, double yoffsetIn);
// 按键事件
void processInput(GLFWwindow *window);

int main() {
    // 初始化
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // 创建窗口
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Learning Chapter08", NULL, NULL);
    if (window == NULL) {
        std::cout << "Create GLFW Window Failed!" << std::endl;
        glfwTerminate();
        return -1;
    }
    // 设置上下文
    glfwMakeContextCurrent(window);
    // 变化窗口尺寸
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // 窗口初始化
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Initial GLFW Windows Failed!" << std::endl;
        return -1;
    }
    // 视口尺寸
    glViewport(0, 0, screenWidth, screenHeight);
	// 垂直同步
	// glfwSwapInterval(1);

	// 检测鼠标移动事件并调用回调函数
	glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// 检测鼠标滚轮事件并调用回调函数
	glfwSetScrollCallback(window, scroll_callback);
	
    // 元素缓冲对象
	float R = 0.8f;
	float a = (2.0f * sqrtf(6.0f) / 3.0f) * R;
	float r = a / sqrtf(3.0f);
	float H = sqrtf(powf(a, 2.0f) - powf(r, 2.0f));
	float h = H - R;
    float vertices[] = { 
		// 		---- 位置 ----									---- 颜色 ----	
		0.0f,		 R,		 0.0f,							0.7f, 0.4f, 0.0f,		// 锥顶P0
		0.0f,		-h,		 r,								0.7f, 0.0f, 0.0f,		// 底面顶点P1
		a / 2.0f,	-h,		-(a / 2.0f) / sqrtf(3.0f),		0.0f, 0.7f, 0.0f,		// 底面顶点P2
		-a / 2.0f,	-h,		-(a / 2.0f) / sqrtf(3.0f),		0.0f, 0.0f, 0.8f		// 底面顶点P3
    };
    
    unsigned int indices[] = {  // 元素索引
		0, 1, 2,
		0, 2, 3,
		0, 1, 3,
		1, 2, 3	
    };

    // 生成配置 VAO、EBO、VBO
    // 绘制面
    unsigned int VAO1, VAO2, EBO1, EBO2, VBO;
    glGenVertexArrays(1, &VAO1);   
    glGenBuffers(1, &EBO1);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // 绘制线
    glGenVertexArrays(1, &VAO2);   
    glGenBuffers(1, &EBO2);
    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // 编译、创建着色器
    Shader myShader("src/shaders/shader.vert","src/shaders/shader.frag");
    myShader.use();
    
    glEnable(GL_DEPTH_TEST);	// 启用深度测试
	
    while (!glfwWindowShouldClose(window)){
		processInput(window);

        // 背景重绘
		glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// 清除屏幕缓冲与深度缓冲
		
        float timeValue = glfwGetTime();
        float colorValue[] = {timeValue, timeValue, timeValue};
        myShader.set3Floatv("time", colorValue);
		
        // 显示实时帧率
		float currentFps = updateFPS();
		if (fabsf(lastFps - currentFps) >= 0.1) {
			lastFps = currentFps;
			std::cout << std::fixed << std::setprecision(1) << lastFps << " fps" << std::endl;
		}
		
        // 设置模型矩阵
        glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.6f, 0.0f));
        // model = glm::rotate(model, timeValue * glm::radians(50.0f), glm::vec3(1.0f, 0.0f, 0.0f));    // 每秒绕 x 轴旋转 50 度
        myShader.set4Mat("model", model);
        
        // 设置 LookAt 矩阵
        myShader.set4Mat("view", myCamera.GetViewMatrix());
		// 设置投影矩阵
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(myCamera.m_zoom), (float)(screenWidth / screenHeight), 0.1f, 100.0f);  // 透视投影
		myShader.set4Mat("projection", projection);
		
        // 绘制面
        glBindVertexArray(VAO1);
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
        // 绘制线
        glBindVertexArray(VAO2);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // 释放缓冲与着色器
	glDeleteVertexArrays(1, &VAO1);
	glDeleteVertexArrays(1, &VAO2);
    glDeleteBuffers(1, &EBO1);
    glDeleteBuffers(1, &EBO2);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(myShader.ID);

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    screenWidth = width;
    screenHeight = height;
}    

float updateFPS() {
    double currentTime = glfwGetTime();
    frameCount++;
    if (currentTime - lastTime >= 1.0) {
        fps = frameCount / (currentTime - lastTime);
        frameCount = 0;
        lastTime = currentTime;
    }
    return fps;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    myCamera.ProcessMouseMovement(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffsetIn, double yoffsetIn) {
    float yoffset = static_cast<float>(yoffsetIn);
    myCamera.ProcessMouseScroll(yoffset);
}

void processInput(GLFWwindow *window) {
    // 按下 WASD 改变摄像机位置
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    Camera::Camera_Movement direction = Camera::Camera_Movement::NOTMOVE;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        direction = Camera::Camera_Movement::FORWARD;
    }    
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        direction = Camera::Camera_Movement::BACKWARD;
    }    
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        direction = Camera::Camera_Movement::LEFT;
    }    
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        direction = Camera::Camera_Movement::RIGHT;
    }

    static bool lastRState = false;
    bool currentRState = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
    if (currentRState && !lastRState) {
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) { // 按 R 恢复 fov
            direction = Camera::Camera_Movement::REZOOM;
            std::cout << "The FOV has been reset to default value." << std::endl;
        }    
    }
	lastRState = currentRState;
	
    myCamera.ProcessKeyboard(direction, deltaTime);

    // 按下 Esc 退出窗口
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }    
}    
