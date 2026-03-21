#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <iomanip>
// 包含 glm 数学库相关头文件
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
    GLFWwindow* window = glfwCreateWindow(800, 800, "Learning Chapter05", NULL, NULL);
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
    glViewport(0, 0, 800, 800);

	// 垂直同步
	glfwSwapInterval(1);

    // 编译、创建着色器
    Shader myShader("src/shaders/shader.vert","src/shaders/shader.frag");

    // 元素缓冲对象
    float vertices[] = { 
        // ---- 位置 ----		---- 颜色 ----	
         0.5f, -0.5f, 0.0f,		0.7f, 0.4f, 0.0f,	// 右下
         0.5f,  0.5f, 0.0f,		0.7f, 0.0f, 0.0f,	// 右上
        -0.5f, -0.5f, 0.0f,		0.0f, 0.7f, 0.0f,	// 左下 
        -0.5f,  0.5f, 0.0f,		0.0f, 0.0f, 0.8f	// 左上    
    };    
    
    unsigned int indices[] = {  // 元素索引
        0, 1, 2, 
        1, 2, 3
    };

	// 生成配置 VAO、EBO、VBO
	unsigned int VAO, EBO, VBO;
    glGenVertexArrays(1, &VAO);   
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
	
    myShader.use();
    while (!glfwWindowShouldClose(window)){
		processInput(window);
        // 背景重绘
		glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
		
        float timeValue = glfwGetTime();
        float colorValue[] = {timeValue, timeValue, timeValue};
        myShader.set3Floatv("time", colorValue);
		
		// 矩阵变换（先位移再缩放后旋转）
		// 生成矩阵即矩阵相乘，将相应参数所代表的矩阵右乘原矩阵，故由代码顺序转写为数学语言时应为从下往上读代码
		glm::mat4 trans = glm::mat4(1.0f);  // 创建一个单位矩阵
		// 生成旋转矩阵（原矩阵，旋转弧度，旋转轴）
		trans = glm::rotate(trans, glm::radians(5000 * timeValue), glm::vec3(0.0,0.0,1.0));    // 绕 Z 轴旋转
		// 生成缩放矩阵（原矩阵，缩放量）
		trans = glm::scale(trans, glm::vec3(fabsf(sinf(1.7 * timeValue) / 3.7) + 0.2f, fabsf(cosf(1.3 * timeValue) / 4.3) + 0.3f, fabsf(sinf(0.7 * timeValue)) / 2.7) + 0.2f);    // 不均匀缩放
		// 生成位移矩阵（原矩阵，位移量）
		trans = glm::translate(trans, glm::vec3(sinf(timeValue) / 2.1f, cosf(timeValue) / 1.7f, sinf(timeValue) / 2.3));
		myShader.set4Mat("transform", trans);	// 复制矩阵值至着色器
		
		// 显示实时帧率
		float currentFps = updateFPS();
		if (fabsf(lastFps - currentFps) >= 0.1) {
			lastFps = currentFps;
			// 格式化显示
			std::cout << std::fixed << std::setprecision(1) << lastFps << " fps" << std::endl;
		}

        // 绘制
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // 释放缓冲与着色器
	glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(myShader.ID);

    glfwTerminate();
    return 0;
}