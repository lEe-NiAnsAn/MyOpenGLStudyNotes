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
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Learning Chapter15", NULL, NULL);
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
	// 垂直同步
	glfwSwapInterval(1);
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
	unsigned int VAO, vao2, EBO, VBO, vbo2;
    glGenVertexArrays(1, &VAO);   
    glGenVertexArrays(1, &vao2);   
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &vbo2);

	glBindVertexArray(VAO);
	// 自定分配缓冲对象
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertPos) + sizeof(vertCol) + sizeof(vertNor), NULL, GL_STATIC_DRAW);	// 申请预留缓冲空间
	
	// 写入数据方法一：从缓冲空间的0字节开始写入 verPos 大小的数据，数据来源亦为 verPos
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertPos), vertPos);	
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertPos), sizeof(vertCol), vertCol);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertPos) + sizeof(vertCol), sizeof(vertNor), vertNor);
 
	// 顶点属性备份对象（同为 GL_ARRAY_BUFFER 时，为防止绑定对象冲突，可将备份设置为 GL_COPY_WRITE_BUFFER）
	glBindBuffer(GL_COPY_WRITE_BUFFER, vbo2);
    glBufferData(GL_COPY_WRITE_BUFFER, sizeof(vertPos) + sizeof(vertCol) + sizeof(vertNor), NULL, GL_STATIC_DRAW);
	// 复制顶点属性数据至备份对象：
	glCopyBufferSubData(GL_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(vertPos) + sizeof(vertCol) + sizeof(vertNor));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	// 设置【代替交错布局】的分批形式的顶点属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
    glEnableVertexAttribArray(0);	// 位置
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vertPos)));
    glEnableVertexAttribArray(1);	// 颜色
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vertPos) + sizeof(vertCol)));
    glEnableVertexAttribArray(2);	// 法线向量
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
	
	// 设置备份顶点属性
	glBindVertexArray(vao2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	// 写入方法二：获取 GPU 内缓冲对象指针映射至 CPU 进行只写录入（glMapBuffer 为整体映射）
	void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(vertPos), GL_MAP_WRITE_BIT);	//  顶点位置属性数据范围映射
	if (ptr) {
		float* data = (float*)ptr;	// 显式转换为目标数据类型数组以修改z值
		data[2]  = 1.0f;
		data[5]  = 1.0f;
		data[8]  = 1.0f;
		data[11] = 1.0f;
		glUnmapBuffer(GL_ARRAY_BUFFER);	// 释放指针
	}
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vertPos)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vertPos) + sizeof(vertCol)));
    glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	auto myShader = std::make_unique<Shader>("src/shaders/shader.vert", "src/shaders/shader.frag");
	// 设置 相应绑定点
	unsigned int matrices_index = glGetUniformBlockIndex(myShader->ID, "Matrices");	// 获取着色器中 uniform 块索引
	glUniformBlockBinding(myShader->ID, matrices_index, 0);	// 设置至0号标志绑定点（自定序号）
	// 设置 uniform 数据缓冲对象
	unsigned int UBO;
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW); // 预分配内存：根据 UBO 的具体数据内容设置内存大小
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	// 绑定 UBO 至绑定点
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, 2 * sizeof(glm::mat4));	// 区间绑定
	// glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);	// 全绑定
	glBindBuffer(GL_UNIFORM_BUFFER, 0);	// 退出 UBO 状态机

	while (!glfwWindowShouldClose(window)){
		processInput(window);
		
		// 背景重绘
        glClearColor(0.2f, 0.2f, 0.2f, 0.2f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 光源设置
		myShader->use();	
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);	    // 设置光源颜色
        glm::vec3 direction =myCamera->m_front;     // 设置光照方向同摄像机所摄方向
        glm::vec3 lightPos = myCamera->m_position;	// 设置光源位置同摄像机位置
        myShader->setVec3("light.direction", direction);
        myShader->setVec3("light.position", lightPos);
        myShader->set1Float("light.innerCutOff", cosf(glm::radians(11.0f)));	// 传入聚光内切光角余弦值
        myShader->set1Float("light.outerCutOff", cosf(glm::radians(13.0f)));	// 传入聚光外切光角余弦值
        myShader->setVec3("light.ambient", glm::vec3(0.4f) * lightColor);		// 传入环境光强分量
        myShader->setVec3("light.diffuse",  glm::vec3(0.5f) * lightColor);  	// 传入漫反射光强分量
        myShader->setVec3("light.specular", glm::vec3(0.4f) * lightColor);		// 传入镜面反射光强分量
        float linear{0.2f}, quadratic{0.022f};                           		// 设置光照衰减函数一次项、二次项
        myShader->set1Float("light.linear", linear);
        myShader->set1Float("light.quadratic", quadratic);
        myShader->set1Float("light.constant", 1.0f);
		
		glm::vec2 screenSize = glm::vec2(screenWidth, screenHeight);
		myShader->setVec2("screenSize", screenSize);	// 传入屏幕尺寸

		glBindBuffer(GL_UNIFORM_BUFFER, UBO);	// 进入 UBO 状态机
		// 透视裁剪
        glm::mat4 projection = glm::perspective(glm::radians(myCamera->m_zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));	// UBO前半部分 写入透视矩阵
		// 观察视角
        glm::mat4 view = myCamera->GetViewMatrix();
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));	// UBO后半部分 写入视角矩阵
		glBindBuffer(GL_UNIFORM_BUFFER, 0);		// 退出 UBO 状态机
		// 位移缩放
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        myShader->setMat4("model", model);
		
		// 绘制物体
		glBindVertexArray(VAO);
        glDrawElements(GL_POINTS, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(vao2);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
        glfwPollEvents();
	}
	// 释放资源
	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &vao2);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &vbo2);
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
	}

    // 按下 Esc 退出窗口
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }   
}