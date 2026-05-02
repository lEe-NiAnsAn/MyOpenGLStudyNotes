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
// 按键事件工具函数：切换至全屏独占模式
void switch2Full(GLFWwindow *window);
// 按键事件工具函数：切换至窗口化模式
void switch2Win(GLFWwindow *window);

bool VSync = false;	// 垂直同步状态
bool isFullscreen = false;	// 是否处于全屏独占模式
int windowedPosX, windowedPosY, windowedWidth, windowedHeight;	// 保存处于窗口化模式时相关数据

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
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Learning Chapter17", NULL, NULL);
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
	// 默认关闭垂直同步
	glfwSwapInterval(VSync);
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
	auto size_vertPos = sizeof(vertPos);
    float vertCol[] = {	// 颜色
		0.99f, 0.55f, 0.23f, 
		0.99f, 0.55f, 0.23f, 
		0.99f, 0.55f, 0.23f, 
		0.99f, 0.55f, 0.23f
	};
	auto size_vertCol = sizeof(vertCol);
    float vertNor[] = {	// 法线
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f
	};
	auto size_vertNor = sizeof(vertNor);
    unsigned int indices[] = {
		0, 1, 2, 
        2, 3, 0
    };    
	auto size_indices = sizeof(indices);

	// 设置 shader2 实例变换矩阵，实现在一个体积环中随机分布
	unsigned int amount = 100000;	// 实例数量十万（使用顶点属性可大幅减轻性能负担）
	glm::mat4 *modelMatrices;	// 变换矩阵数组指针
	modelMatrices = new glm::mat4[amount];	// 堆上创建便于释放内存
	srand(glfwGetTime()); // 传入时间以初始化随机种子    
	float radius = 7.0f;	// 体积环半径
	float offset = 0.3f;	// 浮动参数
	float scale = 0.002f;	// 缩放参数
	glm::vec3 centerPos = glm::vec3(0.0f, 0.0f, 3.25f);	// 环心坐标
	for(unsigned int i = 0; i < amount; i++) {	// 迭代生成变换矩阵
		glm::mat4 model = glm::mat4(1.0f);
		// 位移：以XoY平面单位圆为基础，环径大小浮动 offset ，厚度浮动 0.4*offset
		float angle = (float)i / (float)amount * 360.0f;	// 均分角
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;	// 生成x分量的随机浮动量
		float x = sin(angle) * radius + displacement + centerPos.x;	// 设置移动矩阵的x轴分量
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;	// 生成y分量的随机浮动量
		float y = cos(angle) * radius + displacement + centerPos.y;	// 设置移动矩阵的y轴分量
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;	// 生成z分量的随机浮动量(小于x轴与z轴，实现扁平环)
		float z = displacement * 0.4f + centerPos.z;	// 设置移动矩阵的z轴分量
		model = glm::translate(model, glm::vec3(x, y, z));
		// 缩放
		float scale = (rand() % 2) / 1000.0f + 0.015;	// 生成 (0.002, 0.0035) 区间内的缩放参数
		model = glm::scale(model, glm::vec3(scale));
		// 旋转
		float rotAngle = (rand() % 360);	// 随机旋转某个整数角度
		model = glm::rotate(model, rotAngle, glm::vec3(0.0f, 0.0f, 1.0f));	// 旋转轴为z轴

		modelMatrices[i] = model;
	}  
	glm::vec3 *offsets = new glm::vec3[amount];
	float scale2 = sqrtf(scale);	// 对面积缩放参数开平方得到长度缩放参数
	for(unsigned int i = 0; i < amount; i++) {	// 交替设置偏移量
		if (i % 2 == 0) {
			offsets[i] = glm::vec3(0.0f, 0.0f, 3.0f / scale2);	// 偏移中心同变换矩阵属性，且需考虑缩放大小
		}
		else {
			offsets[i] = glm::vec3(0.0f, 0.0f, -3.0f / scale2);
		}
	}
	
	// 生成 VAO、EBO、VBO
	unsigned int VAO, EBO, VBO;
    glGenVertexArrays(1, &VAO);   
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBO);

	// 绑定 VAO、设置写入 VBO 与 EBO
	glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size_vertPos + size_vertCol + size_vertNor, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size_vertPos, vertPos);	
	glBufferSubData(GL_ARRAY_BUFFER, size_vertPos, size_vertCol, vertCol);
	glBufferSubData(GL_ARRAY_BUFFER, size_vertPos + size_vertCol, size_vertNor, vertNor);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_indices, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	// 设置 shader2 中实例偏移量数组属性
	unsigned int instanceVBO;
	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	auto size_instance = amount * sizeof(glm::vec3);
	glBufferData(GL_ARRAY_BUFFER, size_instance, offsets, GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);	// 传入数组属性至 3 号点位（shader1 中不存在）
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glVertexAttribDivisor(3, 1);	// 显式说明该属性为实例化数组，控制更新数据的时机为渲染一个新实例时（第二个参数为属性除数，默认为0，表示每帧更新）
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// 设置 shader2 中实例变换矩阵数组属性
	unsigned int modelVBO;
	glGenBuffers(1, &modelVBO);
	glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
	auto size_model = amount * sizeof(glm::mat4);
	glBufferData(GL_ARRAY_BUFFER, size_model, modelMatrices, GL_STATIC_DRAW);
	for (unsigned int i = 0; i < 4; i++) {	// 传递顶点属性的大小不可超过 vec4，需要拆分传递
		glEnableVertexAttribArray(4 + i);	// 使用4个连续顶点属性号
		glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));	
		glVertexAttribDivisor(4 + i, 1);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// 设置传入 VBO 数据数组属性
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
    glEnableVertexAttribArray(0);	// 位置
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(size_vertPos));
    glEnableVertexAttribArray(1);	// 颜色
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(size_vertPos + size_vertCol));
    glEnableVertexAttribArray(2);	// 法线向量
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
	
	auto myShader1 = std::make_unique<Shader>("src/shaders/shader1.vert", "src/shaders/shader.frag");
	auto myShader2 = std::make_unique<Shader>("src/shaders/shader2.vert", "src/shaders/shader.frag");
	// 设置相应绑定点
	unsigned int matrices_index1 = glGetUniformBlockIndex(myShader1->ID, "Matrices");
	glUniformBlockBinding(myShader1->ID, matrices_index1, 0);
	unsigned int matrices_index2 = glGetUniformBlockIndex(myShader2->ID, "Matrices");
	glUniformBlockBinding(myShader2->ID, matrices_index2, 0);
	// 设置 uniform 数据缓冲对象
	unsigned int UBO;
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	// 绑定 UBO 至绑定点
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	// 设置光源
	unsigned int lights_index1 = glGetUniformBlockIndex(myShader1->ID, "Light");
	glUniformBlockBinding(myShader1->ID, lights_index1, 1);
	unsigned int lights_index2 = glGetUniformBlockIndex(myShader2->ID, "Light");
	glUniformBlockBinding(myShader2->ID, lights_index2, 1);
	unsigned int uboLight;
	glGenBuffers(1, &uboLight);
	glBindBuffer(GL_UNIFORM_BUFFER, uboLight);
	GLsizeiptr uboLightSize = 5 * sizeof(float) + 5 * (sizeof(glm::vec3) + 4) + 12;
	glBufferData(GL_UNIFORM_BUFFER, uboLightSize, NULL, GL_STATIC_DRAW);	
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboLight);
	void* uboLight_ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, uboLightSize, GL_MAP_WRITE_BIT);
	if (uboLight_ptr) {
		// 传入光源 float 类型参数
		float* floatData = static_cast<float*>(uboLight_ptr);
		float innerCutOff{cosf(glm::radians(11.0f))}, outerCutOff{cosf(glm::radians(13.0f))};	// 设置聚光内外切光角余弦值	
		floatData[0] = innerCutOff;
		floatData[1] = outerCutOff;
		float constant{1.0f}, linear{0.2f}, quadratic{0.022f};	// 设置光照衰减函数常数项、一次项、二次项
		floatData[2] = constant;
		floatData[3] = linear;
		floatData[4] = quadratic;
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// 设置 shader1 中实例坐标偏移量 UBO
	unsigned int offsets_index = glGetUniformBlockIndex(myShader1->ID, "InstanceOffsets");
	glUniformBlockBinding(myShader1->ID, offsets_index, 2);
	unsigned int uboOffsets;
	glGenBuffers(1, &uboOffsets);
	glBindBuffer(GL_UNIFORM_BUFFER, uboOffsets);
	glBufferData(GL_UNIFORM_BUFFER, 125 * (sizeof(glm::vec3) + 4), NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboOffsets);
	void* uboOffsets_ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, 125 * (sizeof(glm::vec3) + 4), GL_MAP_WRITE_BIT);
	if (uboOffsets_ptr) {
		float spacing = 1.5f;	// 偏移步长
		glm::vec3 start = glm::vec3(-3.0f, -3.0f, 0.5f);	// 初始偏移量
		float* offsetsData = static_cast<float*>(uboOffsets_ptr);
		int idx = 0;
		for (int i = 0; i < 5; i++) {	// 迭代设置5*5范围内实例坐标
			float x = start.x + i * spacing;
			for (int j = 0; j < 5; j++) {
				float y = start.y + j * spacing;
				for (int k = 0; k < 5; k++) {
					float z = start.z + k * spacing;
					offsetsData[idx * 4] = x;
					offsetsData[idx * 4 + 1] = y;
					offsetsData[idx * 4 + 2] = z;
					offsetsData[idx * 4 + 3] = 0;	// 填充位
					idx++;
				}
			}
		}
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	while (!glfwWindowShouldClose(window)){
		processInput(window);
		
		// 背景重绘
        glClearColor(0.2f, 0.2f, 0.2f, 0.2f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
        // 传入光源 vec3 类型参数
		glBindBuffer(GL_UNIFORM_BUFFER, uboLight);
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);	    		// 设置光源颜色
        glm::vec3 direction =myCamera->m_front;     		// 设置光照方向同摄像机所摄方向
        glm::vec3 lightPos = myCamera->m_position;			// 设置光源位置同摄像机位置
        glm::vec3 diffuse = glm::vec3(0.5f) * lightColor;	// 设置漫反射光强分量
        glm::vec3 specular = glm::vec3(0.3f) * lightColor;	// 设置镜面反射光强分量
        glm::vec3 ambient = glm::vec3(0.4f) * lightColor;	// 设置环境光强分量
		void* uboLight_ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, uboLightSize, GL_MAP_WRITE_BIT);
		if (uboLight_ptr) {
			char* bytePtr = static_cast<char*>(uboLight_ptr);
			// 填充12字节偏移量对齐后再写入
			memcpy(bytePtr + 							   5 * sizeof(float) + 12, glm::value_ptr(lightPos),  sizeof(glm::vec3));
			memcpy(bytePtr + 	 (sizeof(glm::vec3) + 4) + 5 * sizeof(float) + 12, glm::value_ptr(direction), sizeof(glm::vec3));
			memcpy(bytePtr + 2 * (sizeof(glm::vec3) + 4) + 5 * sizeof(float) + 12, glm::value_ptr(diffuse),   sizeof(glm::vec3));
			memcpy(bytePtr + 3 * (sizeof(glm::vec3) + 4) + 5 * sizeof(float) + 12, glm::value_ptr(specular),  sizeof(glm::vec3));
			memcpy(bytePtr + 4 * (sizeof(glm::vec3) + 4) + 5 * sizeof(float) + 12, glm::value_ptr(ambient),   sizeof(glm::vec3));
			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		// 传入屏幕尺寸
		glm::vec2 screenSize = glm::vec2(screenWidth, screenHeight);
		myShader1->use();	
		myShader1->setVec2("screenSize", screenSize);
		myShader2->use();	
		myShader2->setVec2("screenSize", screenSize);
		
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
		myShader1->use();	
        myShader1->setMat4("model", model);
		// 粒子环随运行时间自转
		myShader2->use();
		float time = glfwGetTime();
		float time_angle = time * glm::radians(5.0f);	// 每秒旋转 3 度
        glm::mat4 time_model = glm::rotate(glm::mat4(1.0f), time_angle, glm::vec3(0.0f, 0.0f, 1.0f));;
		myShader2->setMat4("time_model", time_model);
		
		// 绘制物体
		myShader1->use();
		glBindVertexArray(VAO);
		glDisableVertexAttribArray(3);	// 禁用3号数组属性防止触发缓冲区读取造成额外性能开销
		glDisableVertexAttribArray(4);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 125);	//使用实例化绘制125个正方形
		myShader2->use();
		glEnableVertexAttribArray(3);	// 启用3号数组属性以进行实例偏移量属性的传递
		glEnableVertexAttribArray(4);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, amount);	// 使用实例偏移量复制粒子绘制双环

		glfwSwapBuffers(window);
        glfwPollEvents();
	}
	// 释放资源
	glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
	myShader1.reset();
	myShader2.reset();
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
		// 按 F11 切换全屏独占模式
		static bool lastF11State = false;
		bool currentF11State = glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS;
		if (currentF11State && !lastF11State) {
			if (!isFullscreen) {
				switch2Full(window);	
				isFullscreen = true;
				std::cout << "Switched to fullscreen exclusive mode." << std::endl;
			} 
			else {
				switch2Win(window);		
				isFullscreen = false;
				std::cout << "Switched back to windowed mode." << std::endl;
			}
		}
		lastF11State = currentF11State;

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

		// 按 V 控制垂直同步开闭
		static bool lastVState = false;
		bool currentVState = glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS;
		if (currentVState && !lastVState) {
			if (VSync) {
				VSync = false;
				if (isFullscreen) {
					glfwSwapInterval(VSync);
					std::cout << "The VSync has been disabled." << std::endl;
				}
				else {	// 窗口化模式下必须进入全屏独占模式才可脱离硬件调度强制更改垂直缓冲模式
					switch2Full(window);
					glfwSwapBuffers(window);	// 交换缓冲刷新状态
					glfwSwapInterval(VSync);
					switch2Win(window);
				}	
				std::cout << "The VSync has been disabled." << std::endl;
			}
			else {
				VSync = true;
				if (isFullscreen) {
					glfwSwapInterval(VSync);
				}
				else {
					switch2Full(window);
					glfwSwapBuffers(window);
					glfwSwapInterval(VSync);
					switch2Win(window);
				}	
				std::cout << "The VSync has been enabled." << std::endl;
			}
		}
		lastVState = currentVState;
	}

    // 按下 Esc 退出窗口
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void switch2Full(GLFWwindow *window){
	// 保存初始窗口化状态
	glfwGetWindowPos(window, &windowedPosX, &windowedPosY);
	glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
	// 切换到全屏独占
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();	// 返回用户系统的主显示器
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);	// 获取显示器视频模式，数据包括分辨率宽高、RGB色位深、刷新率
	glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);	// 渲染画面切换至主显示器，即全屏独占显示
	// 重置光标位置
	Camera::firstMouse = true;
	glfwSetCursorPos(window, screenWidth / 2.0f, screenHeight / 2.0f);

}

void switch2Win(GLFWwindow *window){
	// 恢复窗口化模式
	glfwSetWindowMonitor(window, NULL, windowedPosX, windowedPosY, windowedWidth, windowedHeight, 0);
	// 重置光标位置
	Camera::firstMouse = true;
	glfwSetCursorPos(window, screenWidth / 2.0f, screenHeight / 2.0f);
}