#include <iostream>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"

enum Color_Flag {
	COLOR_LIGHT 		= 0, 	// 光源颜色
	COLOR_TETRAHEDRON	= 1,	// 四面体颜色
	COLOR_LINE 			= 2 	// 框线颜色
};
int colorFlagValue;
enum Light_Style {
	SOFT_EDGES			= 3,	// 柔边
	HARD_EDGES			= 4		// 硬边
};
static int lightStyle = HARD_EDGES;	// 默认硬边

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
// 法线向量计算
std::vector<float> GetVerticesNormal(const float* vertices, size_t vertices_length, const unsigned int* indices, size_t indices_length);
std::vector<float> GetVerticesNormal(const float* vertices, size_t vertices_length);

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
    float vertices1[] = { 
		0.0f,		 R,		 0.0f,					        // 锥顶P0
		0.0f,		-h,		 r,						        // 底面顶点P1
		a / 2.0f,	-h,		-(a / 2.0f) / sqrtf(3.0f),      // 底面顶点P2
		-a / 2.0f,	-h,		-(a / 2.0f) / sqrtf(3.0f),      // 底面顶点P3
    };
    float vertices2[] = { 
		0.0f,		 R,		 0.0f,					        // 锥顶P0
		0.0f,		-h,		 r,						        // 底面顶点P1
		a / 2.0f,	-h,		-(a / 2.0f) / sqrtf(3.0f),      // 底面顶点P2
		
		0.0f,		 R,		 0.0f,					        // 锥顶P0
		a / 2.0f,	-h,		-(a / 2.0f) / sqrtf(3.0f),      // 底面顶点P2
		-a / 2.0f,	-h,		-(a / 2.0f) / sqrtf(3.0f),      // 底面顶点P3
		
		0.0f,		 R,		 0.0f,					        // 锥顶P0
		-a / 2.0f,	-h,		-(a / 2.0f) / sqrtf(3.0f),      // 底面顶点P3
		0.0f,		-h,		 r,						        // 底面顶点P1
		
		0.0f,		-h,		 r,						        // 底面顶点P1
		-a / 2.0f,	-h,		-(a / 2.0f) / sqrtf(3.0f),      // 底面顶点P3
		a / 2.0f,	-h,		-(a / 2.0f) / sqrtf(3.0f),      // 底面顶点P2
    };
    unsigned int indices[] = {  // 元素索引
		0, 1, 2,
		0, 2, 3,
		0, 3, 1,
		1, 3, 2	
    };
	
	// 漫反射（柔边）法向量
	size_t vertices1_length = sizeof(vertices1) / sizeof(vertices1[0]);	// 原顶点数组元素个数
	size_t indices_length = sizeof(indices) / sizeof(indices[0]);		// 索引元素个数
	std::vector<float> vertices1_normal;		// 临时顶点容器
	vertices1_normal = GetVerticesNormal(vertices1, vertices1_length, indices, indices_length);	// 计算顶点法线向量
	size_t vertices1_normal_count = vertices1_normal.size();	// 容器长度
	float vertices1_normal_array[vertices1_normal_count];
	for (size_t i = 0; i < vertices1_normal_count; i++) {	// 复制到新的顶点数组
		vertices1_normal_array[i] = vertices1_normal[i];
	}
	// 漫反射（硬边）法向量
	size_t vertices2_length = sizeof(vertices2) / sizeof(vertices2[0]);	// 原顶点数组元素个数
	std::vector<float> vertices2_normal;		// 临时顶点容器
	vertices2_normal = GetVerticesNormal(vertices2, vertices2_length);	// 计算顶点法线向量
	size_t vertices2_normal_count = vertices2_normal.size();	// 容器长度
	float vertices2_normal_array[vertices2_normal_count];
	for (size_t i = 0; i < vertices2_normal_count; i++) {	// 复制到新的顶点数组
		vertices2_normal_array[i] = vertices2_normal[i];
	}

    // 生成配置 VAO、EBO、VBO
    unsigned int VAO1, VAO2, VAOe, VAOLight, EBO1, EBO2, EBOLight, VBO, VBOe, VBOLight;

    // 绘制面（柔边）
    glGenVertexArrays(1, &VAO1);   
    glGenBuffers(1, &EBO1);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1_normal_array), vertices1_normal_array, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
	// 绘制面（硬边）
    glGenVertexArrays(1, &VAOe);   
    glGenBuffers(1, &VBOe);
    glBindVertexArray(VAOe);
    glBindBuffer(GL_ARRAY_BUFFER, VBOe);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2_normal_array), vertices2_normal_array, GL_STATIC_DRAW);
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1_normal_array), vertices1_normal_array, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 绘制光源（六面体）
    float verticesLight[] = { 
		 0.0f,  1.0f,  0.0f,	// 顶锥尖L0
		 1.0f,  0.0f,  0.0f,	// 平面顶点L1
		 0.0f,  0.0f, -1.0f,	// 平面顶点L2
		-1.0f,  0.0f,  0.0f,	// 平面顶点L3
		 0.0f,  0.0f,  1.0f,	// 平面顶点L4
		 0.0f, -1.0f,  0.0f,	// 底锥尖L-1
	};
	unsigned int indicesLight[] = {
		0, 1, 2,
		0, 2, 3,
		0, 3, 4,
		0, 4, 1,
		5, 1, 2,
		5, 2, 3,
		5, 3, 4,
		5, 4, 1,
    };
    glGenVertexArrays(1, &VAOLight);
	glGenBuffers(1, &EBOLight);
	glGenBuffers(1, &VBOLight);
	glBindVertexArray(VAOLight);
	glBindBuffer(GL_ARRAY_BUFFER, VBOLight);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesLight), verticesLight, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOLight);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesLight), indicesLight, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0));
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
    // 编译、创建着色器
    Shader myShader("src/shaders/shader.vert","src/shaders/shader.frag");
    Shader lightShader("src/shaders/lightShader.vert","src/shaders/lightShader.frag");
    
    glEnable(GL_DEPTH_TEST);	// 启用深度测试
	
    while (!glfwWindowShouldClose(window)){
		processInput(window);

        // 背景重绘
		glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// 清除屏幕缓冲与深度缓冲
		
        // 显示实时帧率
		float currentFps = updateFPS();
		if (fabsf(lastFps - currentFps) >= 0.1) {
			lastFps = currentFps;
			std::cout << std::fixed << std::setprecision(1) << lastFps << " fps" << std::endl;
		}

		myShader.use();		// 物体着色器启用
		// 光照渲染
		float time = glfwGetTime();
		glm::vec3 lightPos(3.0f * sinf(time), 1.0f, 3.0f * cosf(time));	// 光源位置
		float lightColor[] = {1.0f, 1.0f, 1.0f};	// 光源色值
		float ambientStrength;
		if (lightStyle == HARD_EDGES) {ambientStrength = 0.3f;}
		if (lightStyle == SOFT_EDGES) {ambientStrength = 0.5f;}	// 环境光照强度
		myShader.set3Floatv("lightColor", lightColor);
		myShader.set1Float("ambientStrength", ambientStrength);
		myShader.setVec3("lightPos", lightPos);	// 传入光源坐标
        myShader.setVec3("viewPos", myCamera.m_position);  // 传入摄像机坐标
        float specularStrength = 0.4f;   // 镜面反射强度
        myShader.set1Float("specularStrength", specularStrength);
		
        // 设置 LookAt 矩阵
        myShader.set4Mat("view", myCamera.GetViewMatrix());
		// 设置投影矩阵
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(myCamera.m_zoom), (float)(screenWidth / screenHeight), 0.1f, 100.0f);  // 透视投影
		myShader.set4Mat("projection", projection);
		
		// 设置物体模型矩阵
        glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.6f, 0.0f));
        myShader.set4Mat("model", model);

		float objectColor[] = {1.0f, 1.0f, 0.0f};
        float lineColor[] = {0.8f, 0.8f, 0.3f};
		myShader.set3Floatv("objectColor", objectColor);
		if (lightStyle == SOFT_EDGES) {	// 绘制面（柔边透射）
            colorFlagValue = COLOR_TETRAHEDRON;
            myShader.set1Int("colorFlag", colorFlagValue);
			glBindVertexArray(VAO1);
			glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
			// 绘制线框	
			colorFlagValue = COLOR_LINE;
			myShader.set1Int("colorFlag", colorFlagValue);
			myShader.set3Floatv("lineColor", lineColor);
			glBindVertexArray(VAO2);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		if (lightStyle == HARD_EDGES) {	// 绘制面（硬边非透）
            colorFlagValue = COLOR_TETRAHEDRON;
            myShader.set1Int("colorFlag", colorFlagValue);
			glBindVertexArray(VAOe);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			// 绘制线框	
			colorFlagValue = COLOR_LINE;
			myShader.set1Int("colorFlag", colorFlagValue);
			myShader.set3Floatv("lineColor", lineColor);
			glBindVertexArray(VAO2);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		glUseProgram(0);	// 解除着色器激活
		
		lightShader.use();	// 光源着色器启用
        lightShader.set4Mat("view", myCamera.GetViewMatrix());
		lightShader.set4Mat("projection", projection);
		// 设置光源模型矩阵
        model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.05f));	// 缩小光源
        lightShader.set4Mat("model", model);
		
		// 绘制光源
        colorFlagValue = COLOR_LIGHT;
		lightShader.set3Floatv("lightColor", lightColor);
        glBindVertexArray(VAOLight);
        glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);
		glUseProgram(0);	// 解除着色器激活

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // 释放缓冲与着色器
	glDeleteVertexArrays(1, &VAO1);
	glDeleteVertexArrays(1, &VAO2);
	glDeleteVertexArrays(1, &VAOe);
	glDeleteVertexArrays(1, &VAOLight);
    glDeleteBuffers(1, &EBO1);
    glDeleteBuffers(1, &EBO2);
    glDeleteBuffers(1, &EBOLight);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &VBOe);
    glDeleteBuffers(1, &VBOLight);
    glDeleteProgram(myShader.ID);
    glDeleteProgram(lightShader.ID);

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
	
    static bool lastLState = false;
    bool currentLState = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;
    if (currentLState && !lastLState) {
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) { // 按 L 变更光源模式
            if (lightStyle == HARD_EDGES) {
				lightStyle = SOFT_EDGES;
			}
			else if (lightStyle == SOFT_EDGES) {
				lightStyle = HARD_EDGES;
			}
            std::cout << "The lighting mode had been changed." << std::endl;
        }    
    }
	lastLState = currentLState;

    myCamera.ProcessKeyboard(direction, deltaTime);

    // 按下 Esc 退出窗口
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }    
}    

std::vector<float> GetVerticesNormal(const float* vertices, size_t vertices_length, const unsigned int* indices, size_t indices_length) {
    size_t vertexCount = vertices_length / 3;	// 顶点数量
    std::vector<glm::vec3> vertexNormals(vertexCount, glm::vec3(0.0f));	// 各顶点法线容器

    for (size_t i = 0; i < indices_length; i += 3) {	// 逐一计算各面法线向量
        unsigned int i1 = indices[i];
        unsigned int i2 = indices[i+1];
        unsigned int i3 = indices[i+2];

        glm::vec3 a(vertices[i1*3], vertices[i1*3+1], vertices[i1*3+2]);
        glm::vec3 b(vertices[i2*3], vertices[i2*3+1], vertices[i2*3+2]);
        glm::vec3 c(vertices[i3*3], vertices[i3*3+1], vertices[i3*3+2]);

        glm::vec3 normal = glm::cross(b - a, c - a);
        float len = glm::length(normal);
        if (len > 0.0f) normal /= len;	// 归一化面法线，消除三角形面积差异
        vertexNormals[i1] += normal;
        vertexNormals[i2] += normal;
        vertexNormals[i3] += normal;	// 累加面法线
    }

    for (auto& n : vertexNormals) {
        if (glm::length(n) > 0.0f)
            n = glm::normalize(n);	// 归一化各顶点法线向量
    }

    std::vector<float> result;
    result.reserve(vertexCount * 6);  // 三顶点坐标 + 三法线向量坐标
    for (size_t v = 0; v < vertexCount; v++) {
        // 顶点
        result.push_back(vertices[v*3]);
        result.push_back(vertices[v*3+1]);
        result.push_back(vertices[v*3+2]);
        // 法线
        result.push_back(vertexNormals[v].x);
        result.push_back(vertexNormals[v].y);
        result.push_back(vertexNormals[v].z);
    }
    return result;
}

std::vector<float> GetVerticesNormal(const float* vertices, size_t vertices_length) {
	size_t vertexCount = vertices_length / 3;	// 顶点数量
    std::vector<glm::vec3> vertexNormals(vertexCount, glm::vec3(0.0f));	// 各顶点法线容器
	
    for (size_t i = 0; i < vertices_length; i += 9) {	// 计算各顶点法线向量
		unsigned int i1 = i;
		unsigned int i2 = i+3;
		unsigned int i3 = i+6;
        glm::vec3 a(vertices[i1], vertices[i1+1], vertices[i1+2]);
        glm::vec3 b(vertices[i2], vertices[i2+1], vertices[i2+2]);
        glm::vec3 c(vertices[i3], vertices[i3+1], vertices[i3+2]);

        glm::vec3 normal = glm::cross(b - a, c - a);
        float len = glm::length(normal);
        if (len > 0.0f) normal /= len;	// 归一化

        vertexNormals[i/3] += normal;
        vertexNormals[i/3+1] += normal;
        vertexNormals[i/3+2] += normal;	// 三顶点共用当前面法向量，各顶点均有数个法向量代表其在数个不同面中的值
    }

	std::vector<float> result;
    result.reserve(vertexCount * 6);
    for (size_t v = 0; v < vertexCount; v++) {
        // 顶点
        result.push_back(vertices[v*3]);
        result.push_back(vertices[v*3+1]);
        result.push_back(vertices[v*3+2]);
        // 法线
        result.push_back(vertexNormals[v].x);
        result.push_back(vertexNormals[v].y);
        result.push_back(vertexNormals[v].z);
    }
    return result;
}