#include <iostream>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "shader.h"
#include "mesh.h"
#include "model.h"

struct FBO {	// 帧缓冲对象结构体
    unsigned int framebuffer;			// 帧缓冲 ID
    unsigned int textureColorbuffer;	// 纹理 ID
    unsigned int rbo;					// 渲染缓冲 ID
};
FBO myFBO;	// 全局定义帧缓冲对象，便于实现尺寸实时变化
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
// 摄像机对象创建(使用智能指针安全释放)
auto myCamera = std::make_unique<Camera>(glm::vec3(0.0f, 0.3f, 3.0f));  
int screenWidth = 1000;
int screenHeight = 800;	// 设置显示宽高
glm::vec2 offset = glm::vec2(1.0f / screenWidth, 1.0f / screenHeight);  // 创建卷积核所需的偏移量（即单个像素在标准化坐标中的宽高）
float Camera::m_cursorX = screenWidth / 2.0f;
float Camera::m_cursorY = screenHeight / 2.0f;	// 鼠标指针位置窗口居中
bool Camera::firstMouse = true;
bool Camera::IsMouseCaptured = true;
int Camera::m_processFlag = 0;

int main() {
	// 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // 创建窗口
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Learning Chapter14", NULL, NULL);
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
    // stbi 翻转y轴
    stbi_set_flip_vertically_on_load(true);
	// 检测鼠标移动事件并调用回调函数
	glfwSetCursorPosCallback(window, mouse_callback);
	// 检测键盘输入时间并调用回调函数
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// 检测鼠标滚轮事件并调用回调函数
	glfwSetScrollCallback(window, scroll_callback);

    // 铺满视口的画面矩形的顶点数组
    float quadVertices[] = {    // 供自定的帧缓冲对象使用
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    auto myShader = std::make_unique<Shader>("src/shaders/shader.vert", "src/shaders/shader.frag");
    auto screenShader = std::make_unique<Shader>("src/shaders/screenShader.vert", "src/shaders/screenShader.frag");
    auto myModel = std::make_unique<Model>("src/models/test.obj");
	
    // 创建帧缓冲对象
    glGenFramebuffers(1, &myFBO.framebuffer);             	// 生成缓冲对象
    glBindFramebuffer(GL_FRAMEBUFFER, myFBO.framebuffer);	// 绑定操作帧缓冲的状态机
	// 创建颜色纹理附件（读写对象：以添加需要进行读写操作的颜色）
    glGenTextures(1, &myFBO.textureColorbuffer);			// 生成
    glBindTexture(GL_TEXTURE_2D, myFBO.textureColorbuffer);	// 绑定
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);	// 分配纹理内存，数据为空（以便接受后续离屏渲染的颜色值），尺寸同视口（无法响应窗口尺寸改变而固定为此值）
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// 线性插值缩小
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// 线性插值放大
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, myFBO.textureColorbuffer, 0);		// 附加该纹理至绑定的帧缓冲对象
	// 创建渲染缓冲对象（只写对象：以添加无需代码操作读取采样、仅硬件调用的自动深度缓冲与模板缓冲）
	// 				  （若需要自定深度缓冲或模板缓冲则需使用同颜色纹理附件的方式进行创建）
    glGenRenderbuffers(1, &myFBO.rbo);				// 生成 
    glBindRenderbuffer(GL_RENDERBUFFER, myFBO.rbo);	// 绑定 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight); 			// 分配24位深度值与8位模板值（同视口的尺寸）的缓冲的内存
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, myFBO.rbo);	// 附加渲染缓冲对象至帧缓冲深度与模板附件
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)							// 检查帧缓冲对象是否完整：必须附加拥有完整的缓冲附件与颜色附件，且样本数同一
	std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);	// 将帧缓冲的绑定切换至默认对象，防止被意外更改

	while (!glfwWindowShouldClose(window)){
		processInput(window);
		
		// 切换绑定至帧缓冲对象
        glBindFramebuffer(GL_FRAMEBUFFER, myFBO.framebuffer);
        glEnable(GL_DEPTH_TEST); 							// 启用深度测试
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);				// 背景颜色设置
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// 清除颜色及深度缓冲

		// 绘制附件颜色纹理（离屏渲染）	
		myShader->use();		
        // 光源设置
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);	    // 设置光源颜色
        glm::vec3 direction =myCamera->m_front;      // 设置光照方向同摄像机所摄方向
        glm::vec3 lightPos = myCamera->m_position;	// 设置光源位置同摄像机位置
        myShader->setVec3("light.direction", direction);
        myShader->setVec3("light.position", lightPos);
        myShader->set1Float("light.innerCutOff", cosf(glm::radians(11.0f)));	// 传入聚光内切光角余弦值
        myShader->set1Float("light.outerCutOff", cosf(glm::radians(13.0f)));	// 传入聚光外切光角余弦值
        myShader->setVec3("light.ambient", glm::vec3(0.15f) * lightColor);		// 传入环境光强分量
        myShader->setVec3("light.diffuse", glm::vec3(0.85f) * lightColor);  	// 传入漫反射光强分量
        myShader->setVec3("light.specular", glm::vec3(0.15f) * lightColor);		// 传入镜面反射光强分量
        myShader->set1Float("material.shininess", 32.0f);						// 传入镜面高光散射度
        float linear{0.2f}, quadratic{0.022f};                           		// 设置光照衰减函数一次项、二次项
        myShader->set1Float("light.linear", linear);
        myShader->set1Float("light.quadratic", quadratic);
        myShader->set1Float("light.constant", 1.0f);

        // 透视裁剪
        glm::mat4 projection = glm::perspective(glm::radians(myCamera->m_zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		// 观察视角
        glm::mat4 view = myCamera->GetViewMatrix();
        myShader->setMat4("projection", projection);
        myShader->setMat4("view", view);
		// 位移缩放
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        myShader->setMat4("model", model);
		
        myModel->Draw(*myShader);

		// 回退绑定至默认帧缓冲，绘制画面矩形
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);				// 关闭深度测试，防止无法显示矩形
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);	// 设置背景颜色（实际不可见，矩形已完全遮蔽）
        glClear(GL_COLOR_BUFFER_BIT);			// 清除颜色缓冲

		// 使用附件颜色纹理作为矩形纹理，显示上一步离屏渲染的内容
        screenShader->use();
		screenShader->set1Int("screenTexture", 0);
		screenShader->set1Int("postPrecessMode", myCamera->m_processFlag);  // 传入后期处理标志
        screenShader->setVec2("offset", offset);    // 传入偏移量
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, myFBO.textureColorbuffer);	
        glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
        glfwPollEvents();
	}
	// 释放资源
	glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
	glDeleteRenderbuffers(1, &myFBO.rbo);				// 释放渲染缓冲对象
	glDeleteTextures(1, &myFBO.textureColorbuffer);		// 释放附件颜色纹理
    glDeleteFramebuffers(1, &myFBO.framebuffer);		// 释放帧缓冲对象

	myModel.reset();
	screenShader.reset();
	myShader.reset();
    glfwTerminate();
	myCamera.reset();
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) { // 传入窗口句柄与尺寸（回调函数）
	// 最小化时不更新，避免传入0尺寸引发错误
	if (width == 0 || height == 0) return;	

    // 更新视口尺寸
	glViewport(0, 0, width, height);
    screenWidth = width;
    screenHeight = height;

	// 更新颜色纹理附件尺寸
    glBindTexture(GL_TEXTURE_2D, myFBO.textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    // 更新渲染缓冲对象尺寸
    glBindRenderbuffer(GL_RENDERBUFFER, myFBO.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    // 更新卷积核偏移量
    offset = glm::vec2(1.0f / width, 1.0f / height);
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
    static bool lastRState = false; // 最后 R 键状态
    bool currentRState = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
    if (currentRState && !lastRState) {
		direction = Camera::Camera_Movement::REZOOM;
		std::cout << "The FOV has been reset to default value." << std::endl;
    }
	lastRState = currentRState;
    myCamera->ProcessKeyboard(direction, delta);

    // 按数字键恢复切换后期处理效果
	static bool lastNumState = false;
	bool currentNumState = false;
	auto num = GLFW_KEY_0;
	for (auto i = GLFW_KEY_0; i <= GLFW_KEY_9; i++) {
		if (glfwGetKey(window, i) == GLFW_PRESS) {
			currentNumState = true;
			num = i - GLFW_KEY_0;
			break;
		}
	}
	if (currentNumState && !lastNumState && (num != Camera::m_processFlag)) {
		Camera::m_processFlag = num;    // 数字键对应后期处理见片段着色器
		std::cout << "The post-processing mode has been changed." << std::endl;
	}
	lastNumState = currentNumState;

	// 按 ~ 切换鼠标捕获模式
    static bool lastGraveState = false;
    bool currentGraveState = glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS;
	if (currentGraveState && !lastGraveState) {
		// 获取当前光标模式
        int cursorMode = glfwGetInputMode(window, GLFW_CURSOR);
		if (cursorMode == GLFW_CURSOR_DISABLED) {	// 捕获状态隐藏锁定切换为释放状态正常显示
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			Camera::IsMouseCaptured = false;
        } 
		else {	// 释放状态正常显示切换为捕获状态隐藏锁定
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			// 鼠标指针归中
            Camera::firstMouse = true;
            Camera::m_cursorX = screenWidth / 2.0f;
            Camera::m_cursorY = screenHeight / 2.0f;
			Camera::IsMouseCaptured = true;
        }
		std::cout << "Cursor mode has been changed." << std::endl;
	}
	lastGraveState = currentGraveState;
	
    // 按下 Esc 退出窗口
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }   
}