#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <iomanip>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

// 窗口随视口尺寸变化
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}
// 按下 Esc 退出
void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

// 获取实时帧率
double lastTime = glfwGetTime();
int frameCount = 0;
float fps, lastFps = 0.0f;
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

int main() {
    // 初始化
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // 创建窗口
	int screenWidth = 800;
	int screenHeight = 800;	// 设置宽高
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Learning Chapter06", NULL, NULL);
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
	glfwSwapInterval(1);
	
    // 元素缓冲对象
	float R = 0.8f;	// 外接球半径
	float a = (2.0f * sqrtf(6.0f) / 3.0f) * R;	// 棱长
	float r = a / sqrtf(3.0f);	// 正四面体底面半径
	float H = sqrtf(powf(a, 2.0f) - powf(r, 2.0f));	// 正四面体的高
	float h = H - R;	// 原点至底面的高
    float vertices[] = { 
		// 		---- 位置 ----									---- 颜色 ----	
		0.0f,		 R,		 0.0f,							0.7f, 0.4f, 0.0f,		// 锥顶P0
		0.0f,		-h,		 r,								0.7f, 0.0f, 0.0f,		// 底面顶点P1
		a / 2.0f,	-h,		-(a / 2.0f) / sqrtf(3.0f),		0.0f, 0.7f, 0.0f,		// 底面顶点P2
		-a / 2.0f,	-h,		-(a / 2.0f) / sqrtf(3.0f),		0.0f, 0.0f, 0.8f		// 底面顶点P3
    };    
    
    unsigned int indices[] = {  // 元素索引
		0, 1, 2,		// 侧面1（P0, P1, P2）
		0, 2, 3,		// 侧面2（P0, P2, P3）
		0, 1, 3,		// 侧面3（P0, P3, P1）
		1, 2, 3			// 底面 （P1, P2, P3）
    };

    glm::vec3 tetrahedronPositions[] = { // 指定⑨个正四面体在全局空间的位置
        glm::vec3( 0.0f,  0.0f,  0.0f), 
        glm::vec3( 2.5f,  2.5f, -5.0f),
        glm::vec3(-2.5f,  2.5f, -5.0f),
        glm::vec3( 2.5f, -2.5f, -5.0f),
        glm::vec3(-2.5f, -2.5f, -5.0f),
        glm::vec3( 0.0f, -4.0f, -10.0f),
        glm::vec3(-4.0f,  0.0f, -10.0f),
        glm::vec3( 0.0f,  4.0f, -10.0f),
        glm::vec3( 4.0f,  0.0f, -10.0f),
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
    
        // 设置观察矩阵，以实现全局空间的顶点坐标变换为观察空间的坐标
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));	// 在右手坐标系中 z 轴向内远离 3 个单位，实现摄像机后退的效果
        // 设置投影矩阵，以实现观察空间的顶点坐标转变为裁剪空间的坐标
        glm::mat4 projection = glm::mat4(1.0f);
        // 使用透视投影（视场角，视口宽高比，平截头体近平面，平截头体远平面）
        projection = glm::perspective(glm::radians(45.0f), (float)(screenWidth / screenHeight), 0.1f, 100.0f);
        myShader.set4Mat("view", view);
        myShader.set4Mat("projection", projection);	// 传入着色器
        
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
        
        // 绘制面
        glBindVertexArray(VAO1);
        for (unsigned int i = 1; i <= 9; i++) {
            // 设置模型矩阵，以实现局部空间的顶点坐标变换至全局空间内坐标
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, tetrahedronPositions[i]); // 选择全局坐标
            float angle = 10.0f * i; 
            model = glm::rotate(model, (float)glfwGetTime() * glm::radians(angle), glm::vec3(0.5f, 1.0f, 0.0f));    // 每秒绕 (0.5,1,0) 轴旋转 10n 度
            myShader.set4Mat("model", model);
            glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
        }
        // 绘制线
        glBindVertexArray(VAO2);
        for (unsigned int i = 1; i <= 9; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, tetrahedronPositions[i]);
            float angle = 10.0f * i; 
            model = glm::rotate(model, (float)glfwGetTime() * glm::radians(angle), glm::vec3(0.5f, 1.0f, 0.0f));
            myShader.set4Mat("model", model);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
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