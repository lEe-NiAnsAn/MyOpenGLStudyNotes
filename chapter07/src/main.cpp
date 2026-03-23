#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <iomanip>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

#define CAM_ORBITING 0  // 摄像机环绕拍摄
#define CAM_MOVE 1  // 摄像机自由移动

// 窗口随视口尺寸变化
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
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

// 按键事件
float deltaTime = 0.0f; // 帧间时差
float lastFrame = 0.0f; // 上一帧时间点
int cameraBehaviour = CAM_ORBITING; // 默认环绕拍摄
glm::vec3 cameraPos2 = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);   // 摄像机所摄方向
glm::vec3 cameraUp2 = glm::vec3(0.0f, 1.0f, 0.0f);
void processInput(GLFWwindow *window) {
    // 按下 R 变更摄像机行为
    static bool lastRState = false;   // 记录上一帧 R 键状态
    bool currentRState = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
    if (currentRState && !lastRState) { // 按下 R 键后的首帧改变行为
        if (cameraBehaviour == CAM_ORBITING){
            cameraBehaviour = CAM_MOVE;
        }    
        else if (cameraBehaviour == CAM_MOVE) {
            cameraBehaviour = CAM_ORBITING;
        }    
        std::cout << "Camera behaviour had been Changed." << std::endl;
    }    
    lastRState = currentRState;
    
    // 按下 WASD 改变摄像机位置
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;   // 计算时间差，使得不同设备的速度相同
    lastFrame = currentFrame;
    float cameraSpeed = 2.5f * deltaTime;  // 摄像机移动速度
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { // 向负z轴方向前进
        cameraPos2 += cameraSpeed * cameraFront;    
    }    
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { // 向负x轴左移
        cameraPos2 -= glm::normalize(glm::cross(cameraFront, cameraUp2)) * cameraSpeed;
    }    
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { // 向正z轴方向后退
        cameraPos2 -= cameraSpeed * cameraFront;
    }    
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { // 向正x轴右移
        cameraPos2 += glm::normalize(glm::cross(cameraFront, cameraUp2)) * cameraSpeed;
    }    
    
    // 按下 Esc 退出窗口
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
	}    
}    

int screenWidth = 800;
int screenHeight = 800;	// 设置显示宽高
float lastX = screenWidth / 2.0f;   // 鼠标上一帧坐标x值
float lastY = screenHeight / 2.0f;  // 鼠标上一帧坐标y值
float pitch = 0.0f; // 俯仰角
float yaw = -90.0f;   // 偏航角  
float fov = 45.0f;  // 视场角
bool firstMouse = true; // 是否首次捕获鼠标
// 鼠标事件
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (cameraBehaviour == CAM_ORBITING) {	// 非当前摄像机行为及时退出程序，并将是否首次捕捉鼠标标志置为初始值
		firstMouse = true;
		return;
	}

	float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);	// 回调函数默认传入的 double 类型转换为 float 类型

    if (firstMouse) {	// 初次获取鼠标时为鼠标坐标赋传入值
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;	// 检查鼠标帧间移动距离
    float yoffset = lastY - ypos;	// y坐标自底部往顶部依次增大的，故相反
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05f;	// 灵敏度
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;	// 添加偏移量

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;	// 设置角度范围

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

int main() {
    // 初始化
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // 创建窗口
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

	// 检测鼠标事件并调用回调函数
	glfwSetCursorPosCallback(window, mouse_callback);
	// 捕捉鼠标及其运动，在窗口中不可见并将其锁定在窗口中心
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
	
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

		// 摄像机环绕拍摄
		float radius = 5.0f;	// 摄像机绕xOz平面轨迹半径
		float camX = sinf(2.0f * time) * radius;	// 摄像机x坐标
		float camY = 0.0f;	// 摄像机y坐标
		float camZ = cosf(2.0f * time) * radius;	// 摄像机z坐标
		glm::vec3 cameraPos1 = glm::vec3(camX, camY, camZ);	// 摄像机位于全局空间的坐标
		// 设置摄像机朝向的反方向（正z轴）
		glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);	// 摄像机所摄目标（全局原点）
		glm::vec3 cameraDirection1 = glm::normalize(cameraPos1 - cameraTarget);	// 归一化坐标差即为方向向量（交换减数与被减数实现所得方向向量由目标指向摄像机，即摄像机的正z轴方向）
		// 设置摄像机的右轴（正x轴）
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);	// 向上单位向量
		glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection1));	// 向上单位向量右叉乘正z轴方向向量即为正x轴方向
		// 设置摄像机的上轴（正y轴）
		glm::vec3 cameraUp1 = glm::cross(cameraDirection1, cameraRight);	// 正z轴方向右叉乘正x轴方向即为正y轴方向
		// 设置观察矩阵
		glm::mat4 view1 = glm::mat4(1.0f);
		// LookAt矩阵：若三个相互垂直的向量定义了一个坐标空间，则利用该坐标系“三轴正方向”所构成的“四行四列矩阵”右乘“平移向量”所构成的“四行四列矩阵”后所得的矩阵（其中平移向量为参数位置向量的反方向）
		// 任一向量乘以LookAt矩阵可将其变换为目标坐标空间中的向量，达到变更全局空间视角至摄像机视角的效果
		view1 = glm::lookAt(cameraPos1, cameraTarget, up);	// 通过同上述的计算得到三个坐标轴正方向以及位置向量的反方向
        
        // 摄像机自由移动

		glm::mat4 view2 = glm::mat4(1.0f);
        view2 = glm::lookAt(cameraPos2, cameraPos2 + cameraFront, cameraUp2);
        
        // 选择摄像机行为
        if (cameraBehaviour == CAM_ORBITING) {
            myShader.set4Mat("view", view1);
        }
        else if (cameraBehaviour == CAM_MOVE) {
            myShader.set4Mat("view", view2);
        }
		
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