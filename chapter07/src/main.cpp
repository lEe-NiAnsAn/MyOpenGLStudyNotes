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
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Learning Chapter07", NULL, NULL);
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
	glm::vec3 tetrahedronPositions[] = { // 两个正四面体在全局空间的位置
        glm::vec3( 0.0f,  0.0f,  0.0f), 	// 有自转
        glm::vec3( 3.5f,  0.0f, -3.5f)		// 无自传
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
        
		float time = glfwGetTime();

		// 设置摄像机位置坐标
		float radius = 5.0f;	// 摄像机绕xOz平面轨迹半径
		float camX = sinf(2.0f * time) * radius;	// 摄像机x坐标
		float camY = 0.0f;	// 摄像机y坐标
		float camZ = cosf(2.0f * time) * radius;	// 摄像机z坐标
		glm::vec3 cameraPos = glm::vec3(camX, camY, camZ);	// 摄像机位于全局空间的坐标
		// 设置摄像机朝向的反方向（正z轴）
		glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);	// 摄像机所摄目标（全局原点）
		glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);	// 归一化坐标差即为方向向量（交换减数与被减数实现所得方向向量由目标指向摄像机，即摄像机的正z轴方向）
		// 设置摄像机的右轴（正x轴）
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);	// 向上单位向量
		glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));	// 向上单位向量右叉乘正z轴方向向量即为正x轴方向
		// 设置摄像机的上轴（正y轴）
		glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);	// 正z轴方向右叉乘正x轴方向即为正y轴方向
		// 设置观察矩阵
		glm::mat4 view = glm::mat4(1.0f);
		// LookAt矩阵：若三个相互垂直的向量定义了一个坐标空间，则利用该坐标系“三轴正方向”所构成的“四行四列矩阵”右乘“平移向量”所构成的“四行四列矩阵”后所得的矩阵（其中平移向量为参数位置向量的反方向）
		// 任一向量乘以LookAt矩阵可将其变换为目标坐标空间中的向量，达到变更全局空间视角至摄像机视角的效果
		view = glm::lookAt(cameraPos, cameraTarget, up);	// 通过同上述的计算得到三个坐标轴正方向以及位置向量的反方向
		myShader.set4Mat("view", view);
		
		// 设置投影矩阵
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(45.0f), (float)(screenWidth / screenHeight), 0.1f, 100.0f);  // 透视投影
		myShader.set4Mat("projection", projection);	// 传入着色器
		
        // 设置模型矩阵
        glm::mat4 model0 = glm::mat4(1.0f);
        glm::mat4 model1 = glm::mat4(1.0f);
		model0 = glm::translate(model0, tetrahedronPositions[0]);
        model0 = glm::rotate(model0, time * glm::radians(50.0f), glm::vec3(1.0f, 0.0f, 0.0f));    // 每秒绕 x 轴旋转 50 度
		model1 = glm::translate(model1, tetrahedronPositions[1]);	// 不自转
        glm::mat4 modelStyles[] = {model0, model1};	// 模型矩阵数组
		
		for (unsigned int i= 0; i < 2; i++) {
			// 绘制面
			myShader.set4Mat("model", modelStyles[i]);
			glBindVertexArray(VAO1);
			glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
			// 绘制线
			glBindVertexArray(VAO2);
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