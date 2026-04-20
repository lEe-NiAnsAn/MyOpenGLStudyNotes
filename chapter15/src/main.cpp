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
static Model::ReflectMode reflectS = Model::DIFFUSE;	// 默认漫反射
// 摄像机对象创建(使用智能指针安全释放)
auto myCamera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f);  
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
    // stbi 翻转y轴
    stbi_set_flip_vertically_on_load(true);
	// 检测鼠标移动事件并调用回调函数
	glfwSetCursorPosCallback(window, mouse_callback);
	// 检测键盘输入时间并调用回调函数
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// 检测鼠标滚轮事件并调用回调函数
	glfwSetScrollCallback(window, scroll_callback);

	float skyboxVertices[] = {	// 天空盒顶点数据         
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
	
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
	
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
	
		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
	
		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,
	
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);   
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	auto myShader = std::make_unique<Shader>("src/shaders/shader.vert", "src/shaders/shader.frag");
    auto myModel = std::make_unique<Model>("src/models/test.obj");
	auto skyboxShader = std::make_unique<Shader>("src/shaders/skybox.vert", "src/shaders/skybox.frag");
	unsigned int skyboxTexture = Model::loadSkyBox("src/models/sky_box.png");	// 加载天空盒贴图
	unsigned int texturesIndex = myModel->getTextureNum();	// 获取已加载的纹理数目
	myShader->set1Int("skybox", texturesIndex);	// 传入纹理以实现渲染天空盒与其反射效果
	glActiveTexture(GL_TEXTURE0 + texturesIndex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);	// 绑定

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
        myShader->set1Float("light.innerCutOff", cosf(glm::radians(10.0f)));	// 传入聚光内切光角余弦值
        myShader->set1Float("light.outerCutOff", cosf(glm::radians(15.0f)));	// 传入聚光外切光角余弦值
        myShader->setVec3("light.ambient", glm::vec3(0.3f) * lightColor);		// 传入环境光强分量
        myShader->setVec3("light.diffuse", glm::vec3(0.5f) * lightColor);  		// 传入漫反射光强分量
        myShader->setVec3("light.specular", glm::vec3(0.1f) * lightColor);		// 传入镜面反射光强分量
        myShader->set1Float("material.shininess", 32.0f);						// 传入镜面高光散射度
        float linear{0.2f}, quadratic{0.022f};                           		// 设置光照衰减函数一次项、二次项
        myShader->set1Float("light.linear", linear);
        myShader->set1Float("light.quadratic", quadratic);
        myShader->set1Float("light.constant", 1.0f);
		// 设置物体对天空盒纹理的反射模式
		myShader->set1Int("reflectMode", myModel->m_reflectMode);	// 传入
		myModel->m_reflectMode = reflectS;
		
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
		
		// 先绘制其他物体，写入深度值
        myModel->Draw(*myShader);
		// 后绘制天空盒，实现使用提前深度测试的方法丢弃天空盒中被遮挡的片段
		glDepthFunc(GL_LEQUAL);	// 更改比较运算符为小于等于，使得深度值恒等于1的天空盒可通过测试
		skyboxShader->use();
		glm::mat4 skyboxView = glm::mat4(glm::mat3(myCamera->GetViewMatrix())); 
		skyboxShader->setMat4("projection", projection);
        skyboxShader->setMat4("view", skyboxView);
		skyboxShader->set1Int("skybox", 0);	// 传入	
		glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);	// 绘制
		glDepthFunc(GL_LESS);	// 恢复默认比较运算符

		glfwSwapBuffers(window);
        glfwPollEvents();
	}
	// 释放资源
	glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
	skyboxShader.reset();
	myModel.reset();
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
		// 获取当前光标模式
        int cursorMode = glfwGetInputMode(window, GLFW_CURSOR);
		if (cursorMode == GLFW_CURSOR_DISABLED) {	// 捕获状态隐藏锁定切换为释放状态正常显示
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			Camera::IsMouseCaptured = false;
        } 
		else {	// 释放状态正常显示切换为捕获状态隐藏锁定
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            Camera::firstMouse = true;
			Camera::IsMouseCaptured = true;
        }
		// 鼠标指针坐标归中
		Camera::m_cursorX = screenWidth / 2.0f;
		Camera::m_cursorY = screenHeight / 2.0f;
		glfwSetCursorPos(window, screenWidth / 2.0f, screenHeight / 2.0f);	// 鼠标拉回窗口

		std::cout << "Cursor mode has been changed." << std::endl;
	}
	lastGraveState = currentGraveState;
	
	if (Camera::IsMouseCaptured) {	// 判断是否应该读取键鼠进行操作
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
			direction = Camera::REZOOM;
			std::cout << "The FOV has been reset to default value." << std::endl;
		}
		lastRState = currentRState;
		myCamera->ProcessKeyboard(direction, delta);

		// 按 F 键切换反射模式
		static bool lastFState = false;
		bool currentFState = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
		if (currentFState && !lastFState) {
			if (reflectS == Model::DIFFUSE)
				reflectS = Model::REFLECT;
			else if (reflectS == Model::REFLECT)
				reflectS = Model::REFRACT;
			else if (reflectS == Model::REFRACT)
				reflectS = Model::DIFFUSE;
			std::cout << "The reflection mode has been switched." << std::endl;
		}
		lastFState = currentFState;
	}

    // 按下 Esc 退出窗口
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }   
}