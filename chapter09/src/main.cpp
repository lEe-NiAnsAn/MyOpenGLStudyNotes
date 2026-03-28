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

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "shader.h"
#include "camera.h"

enum Color_Flag {
	COLOR_LIGHT 		= 0, 	// 光源颜色
	COLOR_TETRAHEDRON	= 1,	// 四面体颜色
	COLOR_LINE 			= 2 	// 框线颜色
};
enum Light_Flag {
	POINT_LIGHT     	= 3,	// 点光源
	PARALLELT_LIGHT		= 4, 	// 平行光源
	SPOT_LIGHT        	= 5, 	// 聚光源
};
int colorFlagValue;
int lightFlagValue;
bool Camera::m_fly = false;			    // 默认不可飞行
static int lightMode = POINT_LIGHT;		// 默认点光源

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
std::vector<float> GetVerticesNormal(const float* vertices, size_t vertices_length);

int main() {
	// 初始化
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // 创建窗口
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Learning Chapter09", NULL, NULL);
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
	
	// 编译、创建着色器
	Shader myShader("src/shaders/shader.vert","src/shaders/shader.frag");
	Shader lightShader("src/shaders/lightShader.vert","src/shaders/lightShader.frag");
	myShader.use();

    // 创建漫反射与环境光纹理
    unsigned int diffuseMap;
    glGenTextures(1, &diffuseMap);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);	// 绑定纹理
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// S 轴方向默认重复纹理
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);	// T 轴方向默认重复纹理
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// 线性过滤缩小
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// 线性过滤放大
    // 载入图像1
    int width1, height1, nrChannels1;
    stbi_set_flip_vertically_on_load(true); // 翻转y轴
    unsigned char *data = stbi_load("src/textures/warning_diffuse.png", &width1, &height1, &nrChannels1, 0);
    if(data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);	// 附着纹理
        glGenerateMipmap(GL_TEXTURE_2D);	// 生成多级渐远纹理
    }
    else {
        std::cout << "Loading texture error!" << std::endl;
    }
	stbi_image_free(data);						// 释放图像1内存
	// 创建镜面反射光纹理
    unsigned int specularMap;
    glGenTextures(1, &specularMap);
    glBindTexture(GL_TEXTURE_2D, specularMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 载入图像2
    int width2, height2, nrChannels2;
    stbi_set_flip_vertically_on_load(true);
    data = stbi_load("src/textures/warning_specular.png", &width2, &height2, &nrChannels2, 0);
    if(data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);	// 生成多级渐远纹理
    }
    else {
        std::cout << "Loading texture error!" << std::endl;
    }
    stbi_image_free(data);						// 释放图像2内存
	// 创建放射光纹理
    unsigned int emissionMap;
    glGenTextures(1, &emissionMap);
    glBindTexture(GL_TEXTURE_2D, emissionMap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // 载入图像3
    int width3, height3, nrChannels3;
    stbi_set_flip_vertically_on_load(true);
    data = stbi_load("src/textures/warning_emission.png", &width3, &height3, &nrChannels3, 0);
    if(data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width3, height3, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);	// 生成多级渐远纹理
    }
    else {
        std::cout << "Loading texture error!" << std::endl;
    }
    stbi_image_free(data);						// 释放图像3内存

	glActiveTexture(GL_TEXTURE0);				// 激活纹理1
	glBindTexture(GL_TEXTURE_2D, diffuseMap);	// 绑定纹理1
    glActiveTexture(GL_TEXTURE1);				// 激活纹理2
    glBindTexture(GL_TEXTURE_2D, specularMap);	// 绑定纹理2
    glActiveTexture(GL_TEXTURE2);				// 激活纹理3
    glBindTexture(GL_TEXTURE_2D, emissionMap);	// 绑定纹理3
	// 绑定全部纹理后传入
	myShader.set1Int("material.diffuse", 0);
	myShader.set1Int("material.specular", 1);
	myShader.set1Int("material.emission", 2);
	
    // 元素缓冲对象
	float R = 0.8f;
	float a = (2.0f * sqrtf(6.0f) / 3.0f) * R;
	float r = a / sqrtf(3.0f);
	float H = sqrtf(powf(a, 2.0f) - powf(r, 2.0f));
	float h = H - R;
	float t01 = 0.5f;
	float t02 = 1.0f;
	float t11 = 0.0f;
	float t12 = 0.0f;
	float t21 = 1.0f;
	float t22 = 0.0f;
    float vertices[] = {
	//		------ 顶点坐标 ------					  ------ 纹理坐标 ------
		0.0f,		 R,		 0.0f,					  		t01, t02,     	// 锥顶P0
		0.0f,		-h,		 r,						  		t11, t12,     	// 底面顶点P1
		a / 2.0f,	-h,		-(a / 2.0f) / sqrtf(3.0f),		t21, t22,     	// 底面顶点P2

		0.0f,		 R,		 0.0f,					  		t01, t02,     	// 锥顶P0
		a / 2.0f,	-h,		-(a / 2.0f) / sqrtf(3.0f),		t11, t12,     	// 底面顶点P2
		-a / 2.0f,	-h,		-(a / 2.0f) / sqrtf(3.0f),		t21, t22,     	// 底面顶点P3

		0.0f,		 R,		 0.0f,					  		t01, t02,     	// 锥顶P0
		-a / 2.0f,	-h,		-(a / 2.0f) / sqrtf(3.0f),		t11, t12,     	// 底面顶点P3
		0.0f,		-h,		 r,						  		t21, t22,     	// 底面顶点P1
 
		0.0f,		-h,		 r,						  		t01, t02,     	// 底面顶点P1
		-a / 2.0f,	-h,		-(a / 2.0f) / sqrtf(3.0f),		t11, t12,     	// 底面顶点P3
		a / 2.0f,	-h,		-(a / 2.0f) / sqrtf(3.0f),		t21, t22    	// 底面顶点P2
    };

	// 漫反射法向量
	size_t vertices_length = sizeof(vertices) / sizeof(vertices[0]);	// 原顶点数组元素个数
	std::vector<float> vertices_normal;		// 临时顶点容器
	vertices_normal = GetVerticesNormal(vertices, vertices_length);	// 计算顶点法线向量
	size_t vertices_normal_count = vertices_normal.size();	// 容器长度
	float vertices_normal_array[vertices_normal_count];
	for (size_t i = 0; i < vertices_normal_count; i++) {	// 复制到新的顶点数组
		vertices_normal_array[i] = vertices_normal[i];
	}

    // 生成配置 VAO、EBO、VBO
    unsigned int VAO, VAOLight, EBOLight, VBO, VBOLight;

    glGenVertexArrays(1, &VAO);   
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_normal_array), vertices_normal_array, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
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
        
		myShader.use();
        // 光照渲染
        myShader.setVec3("viewPos", myCamera.m_position);  // 传入摄像机坐标
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);	// 设置光源颜色
        glm::vec3 lightPos;
        glm::vec3 direction;
		float linear, quadratic; 	// 光照衰减函数一次项、二次项
        if (lightMode == POINT_LIGHT) {	// 点光源
            float time = glfwGetTime();
            float a = 0.9f;  // x 轴半轴长
            float b = 2.0f;  // z 轴半轴长
            lightPos = glm::vec3(a * cosf(1.3f * time), 1.0f, b * sinf(1.3f * time) - 1.0f);	// 设置光源位置（中心为(0,-1)的椭圆）
            myShader.setVec3("light.position", lightPos);                       // 传入光源位置
			linear = 0.22f;
			quadratic = 0.2f;
        }
        if (lightMode == PARALLELT_LIGHT) {	// 平行光源
            direction = glm::vec3(0.0f, -0.8f, 1.0f);   // 设置光照方向
            lightPos = glm::vec3(10.1f, 3.0f, -10.5f);	// 设置光源位置
            myShader.setVec3("light.direction", direction);
            myShader.setVec3("light.position", lightPos);	// 传入光源位置
			linear = 0.014f;
			quadratic = 0.0007f;
        }
        if (lightMode == SPOT_LIGHT) {	// 聚光源
            direction =myCamera.m_front;   // 设置光照方向同摄像机所摄方向
            lightPos = myCamera.m_position;	// 设置光源位置同摄像机位置
            myShader.setVec3("light.direction", direction);
            myShader.setVec3("light.position", lightPos);	// 传入光源位置
			myShader.set1Float("light.innerCutOff", cosf(glm::radians(11.0f)));	// 传入聚光内切光角余弦值
			myShader.set1Float("light.outerCutOff", cosf(glm::radians(13.0f)));	// 传入聚光外切光角余弦值
			linear = 0.045f;
			quadratic = 0.075f;
        }
        myShader.set1Int("lightMode", lightMode);
        // 使用结构体名为前缀传入结构体各成员数据
        myShader.setVec3("light.diffuse", glm::vec3(0.8f) * lightColor);	// 传入漫反射光强分量
        myShader.setVec3("light.specular", lightColor);						// 传入镜面反射光强分量
        myShader.setVec3("light.ambient", glm::vec3(0.2f) * lightColor);	// 传入环境光强分量
        myShader.set1Float("material.shininess", 32.0f);					// 传入镜面高光散射度
        // 设置光照衰减参数
        myShader.set1Float("light.constant", 1.0f);
        myShader.set1Float("light.linear", linear);
        myShader.set1Float("light.quadratic", quadratic);
        
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
        
		// 绘制面
		colorFlagValue = COLOR_TETRAHEDRON;
		myShader.set1Int("colorFlag", colorFlagValue);
		lightFlagValue = COLOR_TETRAHEDRON;
		myShader.set1Int("lightFlag", lightFlagValue);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glUseProgram(0);	// 解除着色器激活

        if (lightMode == POINT_LIGHT) {
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
            lightShader.setVec3("lightColor", lightColor);
            glBindVertexArray(VAOLight);
            glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);
            glUseProgram(0);	// 解除着色器激活
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // 释放缓冲与着色器
	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &VAOLight);
    glDeleteBuffers(1, &EBOLight);
    glDeleteBuffers(1, &VBO);
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
    // 按下 WASD ↑↓ 改变摄像机位置
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
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        direction = Camera::Camera_Movement::UP;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        direction = Camera::Camera_Movement::DOWN;
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

    static bool lastFState = false;
    bool currentFState = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
    if (currentFState && !lastFState) {
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) { // 按 F 变更移动模式
            if (Camera::m_fly) {
				Camera::m_fly = false;
			}
			else {
				Camera::m_fly = true;
			}
            std::cout << "The moving mode had been changed." << std::endl;
        }    
    }
	lastFState = currentFState;

    static bool lastLState = false;
    bool currentLState = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;
    if (currentLState && !lastLState) {
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) { // 按 L 变更光源类型
            if (lightMode == POINT_LIGHT) {
                lightMode = PARALLELT_LIGHT;
            }
            else if (lightMode == PARALLELT_LIGHT) {
                lightMode = SPOT_LIGHT;
            }
            else if (lightMode == SPOT_LIGHT) {
                lightMode = POINT_LIGHT;
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

std::vector<float> GetVerticesNormal(const float* vertices, size_t vertices_length) {
	size_t vertexCount = vertices_length / 5;	// 顶点数量（包括纹理坐标）
    std::vector<glm::vec3> vertexNormals(vertexCount, glm::vec3(0.0f));	// 各顶点法线容器
	
    for (size_t i = 0; i < vertices_length; i += 15) {	// 计算各顶点法线向量
		unsigned int i1 = i;
		unsigned int i2 = i+5;
		unsigned int i3 = i+10;
        glm::vec3 a(vertices[i1], vertices[i1+1], vertices[i1+2]);
        glm::vec3 b(vertices[i2], vertices[i2+1], vertices[i2+2]);
        glm::vec3 c(vertices[i3], vertices[i3+1], vertices[i3+2]);

        glm::vec3 normal = glm::cross(b - a, c - a);
        float len = glm::length(normal);
        if (len > 0.0f) normal /= len;	// 归一化

        vertexNormals[i/5] += normal;
        vertexNormals[i/5+1] += normal;
        vertexNormals[i/5+2] += normal;	// 三顶点当前面法向量
    }

	std::vector<float> result;
    result.reserve(vertexCount * 8);
    for (size_t v = 0; v < vertexCount; v++) {
        // 坐标（包括顶点与纹理）
        result.push_back(vertices[v*5]);
        result.push_back(vertices[v*5+1]);
        result.push_back(vertices[v*5+2]);
        result.push_back(vertices[v*5+3]);
        result.push_back(vertices[v*5+4]);
        // 法线向量
        result.push_back(vertexNormals[v].x);
        result.push_back(vertexNormals[v].y);
        result.push_back(vertexNormals[v].z);
    }
    return result;
}