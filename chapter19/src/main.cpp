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
void switch2FullWin(GLFWwindow *window);
// 按键事件工具函数：切换至普通窗口模式
void switch2CommonWin(GLFWwindow *window);
// 按键事件工具函数：切换至无边框模式
void switch2FramelessWin(GLFWwindow *window);
// sRGB 色彩空间色值转换为线性色值
float sRGB2Linear(float u);
void sRGB2Linear(float* colors, int count);

struct FBO {	// 帧缓冲对象结构体
    unsigned int framebuffer = 0;			// 帧缓冲 ID
    unsigned int textureColorbuffer = 0;	// 纹理 ID
    unsigned int rbo = 0;					// 渲染缓冲 ID
};
// 清理帧缓冲对象函数
void cleanupFBO(FBO& fbo);
// 创建多重采样帧缓冲
void setMSAA(FBO& multiple, FBO& intermediate, int multiNum);
FBO multiFBO, intermediateFBO;
int multiNum = 0;	// 多重采样倍数（默认不开启）

bool VSync = false;	// 垂直同步状态
enum WindowMode {CommonWin, Fullscreen, Frameless} windowMode;	// 窗口显示模式枚举
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
	windowMode = WindowMode::CommonWin;	// 初始为有边框普通窗口，确保存在窗口状态值
    // 创建窗口
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Learning Chapter19", NULL, NULL);
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
	// 检测鼠标移动事件并调用回调函数
	glfwSetCursorPosCallback(window, mouse_callback);
	// 检测键盘输入时间并调用回调函数
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// 检测鼠标滚轮事件并调用回调函数
	glfwSetScrollCallback(window, scroll_callback);

	// 视口矩形顶点数组
    float quadVertices[] = {
		// X	  Y		 U	   V
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
	// 设置并传入数组
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
	auto screenShader = std::make_unique<Shader>("src/shaders/screenShader.vert", "src/shaders/screenShader.frag");

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
	// 若颜色值由屏幕取色确定，则该颜色值已是在 sRGB 颜色空间进行了伽马矫正的色值，在程序中手动开启伽马矫正时需手动还原该色值至线性值
	sRGB2Linear(vertCol, size_vertCol / sizeof(vertCol[0]));
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
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_indices, indices, GL_STATIC_DRAW);
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
	
	auto myShader = std::make_unique<Shader>("src/shaders/shader.vert", "src/shaders/shader.frag");
	// 设置相应绑定点
	unsigned int matrices_index = glGetUniformBlockIndex(myShader->ID, "Matrices");
	glUniformBlockBinding(myShader->ID, matrices_index, 0);
	// 设置 uniform 数据缓冲对象
	unsigned int UBO;
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	// 绑定 UBO 至绑定点
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	// 设置光源
	unsigned int lights_index = glGetUniformBlockIndex(myShader->ID, "Light");
	glUniformBlockBinding(myShader->ID, lights_index, 1);
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

	setMSAA(multiFBO, intermediateFBO, multiNum);	// 设置帧缓冲

	float last_time = 0.0f, bg_color = 0.0f;
	while (!glfwWindowShouldClose(window)){
		processInput(window);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		float current_time = glfwGetTime();
		if ((current_time - last_time) > 1.0f) {			// 每隔一秒变化一次背景颜色
			if (bg_color > 1.0f) bg_color = 0.0f;
			else bg_color += 0.1f;
			last_time = current_time;
		}
		// 绑定至多重采样帧缓冲对象（或普通帧缓冲）
        glBindFramebuffer(GL_FRAMEBUFFER, multiFBO.framebuffer);
        glClearColor(bg_color, bg_color, bg_color, 1.0f);	// 背景颜色设置
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// 清除颜色及深度缓冲
        glEnable(GL_DEPTH_TEST); 							// 启用深度测试
		
		myShader->use();	
        // 传入光源 vec3 类型参数
		glBindBuffer(GL_UNIFORM_BUFFER, uboLight);
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);	    		// 设置光源颜色
		glm::vec3 diffuse = glm::vec3(0.5f) * lightColor;	// 设置漫反射光强分量
        glm::vec3 specular = glm::vec3(0.5f) * lightColor;	// 设置镜面反射光强分量
        glm::vec3 ambient = glm::vec3(0.2f) * lightColor;	// 设置环境光强分量
        glm::vec3 direction = myCamera->m_front;     		// 设置光照方向同摄像机所摄方向
		glm::vec3 lightPos = myCamera->m_position;			// 设置光源位置同摄像机位置移动
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
		myShader->setVec2("screenSize", screenSize);
		// 传入摄像机位置
		myShader ->setVec3("cameraPos", myCamera->m_position);
		
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
		// 绘制物体至帧缓冲对象
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		if (multiNum != 0) {
			// 将已经绘制的多重采样帧缓冲数据还原写入中间帧缓冲对象
			glBindFramebuffer(GL_READ_FRAMEBUFFER, multiFBO.framebuffer);			// 绑定读取对象
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO.framebuffer);	// 绑定写入对象
			glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);	// 位块传送缓冲帧数据
		}
        
		// 切换至默认帧缓冲
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);	// 设置背景颜色（实际不可见）
        glClear(GL_COLOR_BUFFER_BIT);			// 清除颜色缓冲
        glDisable(GL_DEPTH_TEST);				// 关闭深度测试
		// 绘制离屏渲染内容
		screenShader->use();
		screenShader->set1Int("multiNum", multiNum);	// 传入采样数
		// 伽马值：设备输出亮度等于电压的 Gamma 次幂
		// （任何设备 Gamma 基本上都不会等于1，等于1是一种理想的线性状态）
		// （当Gamma = 1 时：若电压和亮度均在0到1的区间，则电压就等于亮度值）
		// 早年间的 CRT 显示器伽马值大多为2.2，接近人眼识别亮度的特征，但我们不关注“拟真”，只要求亮度值可按照“预期”代码般线性变化；
		// 现今 Gamma2.2 相当于一个显示器标准，但各个显示器的实际伽马值并非2.2，故需要手动微调矫正。
		// 由于不同显示屏的对同一个颜色的实际显示各异，与其依赖显示屏的 sRGB 等色彩空间进行伽马值矫正，不如首先还原至线性色值后通过各异地微调伽马值来输出用户满意的颜色输出
		// 若我们需要对传入的颜色向量做线性乘除运算，则必须先将其转换至线性空间后再传入着色器进行运算，如此方可避免非线性的 sRGB 空间颜色值在叠加线性系数时造成的颜色漂移
		// 对于传入的纹理图像文件，OpenGL 提供了 GL_SRGB 和 GL_SRGB_ALPHA 两种内部纹理格式，当读取文件时将其设为纹理格式即可实现自动转换至线性值
		// 方案一：使用片段着色器手动计算矫正（只对输入到着色器中的像素进行矫正，由 GL 所设置的背景颜色值不会矫正）
		// 方案二：使用OpenGL内建的sRGB帧缓冲，每次绘制时硬件自动处理，把颜色缓冲中（所有像素）的线性颜色转换为 sRGB 空间，只能在最末帧缓冲渲染时调用一次
		screenShader->set1Bool("gammaFlag", myCamera->m_gammaFlag);	// 传入伽马矫正标志
		screenShader->set1Float("gammaValue", myCamera->m_gammaValue);	// 传入伽马矫正大小
        glBindVertexArray(quadVAO);
		// glEnable(GL_FRAMEBUFFER_SRGB);
		if (multiNum == 0)
			glBindTexture(GL_TEXTURE_2D, multiFBO.textureColorbuffer);	// 直接使用普通采样帧缓冲
		else
			glBindTexture(GL_TEXTURE_2D, intermediateFBO.textureColorbuffer);	// 使用中间帧缓冲
        glDrawArrays(GL_TRIANGLES, 0, 6);
		// glDisable(GL_FRAMEBUFFER_SRGB);	// 及时关闭

		glfwSwapBuffers(window);
        glfwPollEvents();
	}
	// 释放资源
	glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
	glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &UBO);
	glDeleteBuffers(1, &uboLight);
	cleanupFBO(multiFBO);
	cleanupFBO(intermediateFBO);
	myShader.reset();
	screenShader.reset();
    glfwTerminate();
	myCamera.reset();
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) { // 传入窗口句柄与尺寸（回调函数）
	if (width == 0 || height == 0) return;	
	glViewport(0, 0, width, height);
    screenWidth = width;
    screenHeight = height;
	// 更新帧缓冲尺寸
	if (multiNum == 0) {
		glBindTexture(GL_TEXTURE_2D, multiFBO.textureColorbuffer);	
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, multiFBO.rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

	}
	else {
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multiFBO.textureColorbuffer);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multiNum, GL_RGB, screenWidth, screenHeight, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, multiFBO.rbo); 
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, multiNum, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, intermediateFBO.textureColorbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) { // 传入窗口句柄与鼠标坐标（回调函数）
    if (!Camera::IsMouseCaptured) return;
	float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    myCamera->ProcessMouseMovement(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffsetIn, double yoffsetIn) {  // 传入窗口句柄与滚轮运动参数（回调函数）
    if (!Camera::IsMouseCaptured) return;
	float yoffset = static_cast<float>(yoffsetIn);
    myCamera->ProcessMouseScroll(yoffset);
}

void processInput(GLFWwindow *window) { // 传入窗口句柄，实时监测按键事件
	float currentFrame = glfwGetTime();
	delta = currentFrame - lastFrame;
	lastFrame = currentFrame;
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
			if (windowMode == WindowMode::Fullscreen) {
				switch2CommonWin(window);
				std::cout << "Switched to common window mode." << std::endl;
			} 
			else {
				switch2FullWin(window);	
				std::cout << "Switched to fullscreen window mode." << std::endl;
			}
		}
		lastF11State = currentF11State;

		// 按 Alt + Enter 切换无边框模式
		static bool lastAltEnterState = false;
		bool currentleftAltState = glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
		bool currentrightAltState = glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;
		bool currentEnterState = glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;
		bool currentAltEnterState = (currentleftAltState || currentrightAltState) && currentEnterState;
		if (currentAltEnterState && !lastAltEnterState) {
			if (windowMode == WindowMode::Frameless) {
				switch2CommonWin(window);
				std::cout << "Switched to common window mode." << std::endl;
			} 
			else {
				switch2FramelessWin(window);
				std::cout << "Switched to frameless window mode." << std::endl;
			}
		}
		lastAltEnterState = currentAltEnterState;

		// 按下 WASD↑↓ 改变摄像机位置
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
		myCamera->ProcessKeyboard(direction, delta);
	
		// 按 R 恢复 fov
		static bool lastRState = false;
		bool currentRState = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
		if (currentRState && !lastRState) {
			direction = Camera::REZOOM;
			std::cout << "The FOV has been reset to default value." << std::endl;
		}
		lastRState = currentRState;

		// 按 G 切换伽马矫正开启状态
		static bool lastGState = false;
		bool currentGState = glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS;
		if (currentGState && !lastGState) {
			if (!myCamera->m_gammaFlag) {
				myCamera->m_gammaFlag = true;
				std::cout << "The Gamma Correction has been enabled." << std::endl;
			}
			else {
				myCamera->m_gammaFlag = false;
				std::cout << "The Gamma Correction has been disabled." << std::endl;
			}
			myCamera->m_gammaValue = 2.2f;	// 伽马值重置
		}
		lastGState = currentGState;
		
		// 按下 ←→ 改变伽马矫正大小
		if (myCamera->m_gammaFlag) {
			static auto lastLEFTState{GLFW_RELEASE}, lastRIGHTState{GLFW_RELEASE};
			auto currentLEFTState = glfwGetKey(window, GLFW_KEY_LEFT);	
			auto currentRIGHTState = glfwGetKey(window, GLFW_KEY_RIGHT);	
			Camera::Camera_Gamma gamma = Camera::Camera_Gamma::NOTCHANGE;   // 默认不更改
			if (currentLEFTState == GLFW_PRESS) {    // ← 减小
				gamma = Camera::Camera_Gamma::DECREASE;
			}
			if (currentRIGHTState == GLFW_PRESS) {  // → 增大
				gamma = Camera::Camera_Gamma::INCREASE;
			}
			if ((lastLEFTState == GLFW_PRESS && currentLEFTState == GLFW_RELEASE) || 
				(lastRIGHTState == GLFW_PRESS && currentRIGHTState == GLFW_RELEASE)) {	// 松开按键显示当前伽马值
				std::cout << "The Gamma Value is " << myCamera->m_gammaValue << " right now." << std::endl;
			}
			lastLEFTState = currentLEFTState;
			lastRIGHTState = currentRIGHTState; 
			myCamera->ProcessGamma(gamma, delta);
		}
	
		// 按 X 循环更改多重采样数
		static bool lastXState = false;
		bool currentXState = glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS;
		if (currentXState && !lastXState) {
			if (multiNum < 16 && multiNum != 0) {
				multiNum *= 2;
				setMSAA(multiFBO, intermediateFBO, multiNum);
			}
			else if (multiNum == 0) {
				multiNum = 2;
				setMSAA(multiFBO, intermediateFBO, multiNum);
			}
			else {
				multiNum = 0;
				setMSAA(multiFBO, intermediateFBO, multiNum);
			}
			if (multiNum == 0) std::cout << "MSAA has been closed." << std::endl;
			else std::cout << "Changed to " << multiNum << "x MSAA." << std::endl;
		}
		lastXState = currentXState;

		// 按 V 控制垂直同步开闭
		static bool lastVState = false;
		bool currentVState = glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS;
		if (currentVState && !lastVState) {
			if (VSync) {
				VSync = false;
				if (windowMode == WindowMode::Fullscreen) {
					glfwSwapInterval(VSync);
				}
				else if (windowMode == WindowMode::CommonWin) {
					switch2FullWin(window);
					glfwSwapBuffers(window);	// 交换缓冲以刷新状态
					glfwSwapInterval(VSync);
					switch2CommonWin(window);
				}
				else {
					switch2FullWin(window);
					glfwSwapBuffers(window);
					glfwSwapInterval(VSync);
					switch2FramelessWin(window);
				}
				std::cout << "The VSync has been disabled." << std::endl;
			}
			else {
				VSync = true;
				if (windowMode == WindowMode::Fullscreen) {
					glfwSwapInterval(VSync);
				}
				else if (windowMode == WindowMode::CommonWin) {
					switch2FullWin(window);
					glfwSwapBuffers(window);
					glfwSwapInterval(VSync);
					switch2CommonWin(window);
				}
				else {
					switch2FullWin(window);
					glfwSwapBuffers(window);
					glfwSwapInterval(VSync);
					switch2FramelessWin(window);
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

void switch2FullWin(GLFWwindow *window){
	// 保存初始窗口化状态
	if (windowMode == WindowMode::CommonWin) {	// 仅储存普通窗口的状态
		glfwGetWindowPos(window, &windowedPosX, &windowedPosY);
		glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
	}
	// 切换到全屏独占
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	windowMode = WindowMode::Fullscreen;
	// 重置光标位置
	Camera::firstMouse = true;
	glfwSetCursorPos(window, mode->width / 2.0f, mode->height / 2.0f);
}

void switch2CommonWin(GLFWwindow *window){
	// 恢复窗口化模式
	glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);	// 开启窗口装饰
	glfwSetWindowMonitor(window, NULL, windowedPosX, windowedPosY, windowedWidth, windowedHeight, 0);
	windowMode = WindowMode::CommonWin;
	// 重置光标位置
	Camera::firstMouse = true;
	glfwSetCursorPos(window, windowedWidth / 2.0f, windowedHeight / 2.0f);
}

void switch2FramelessWin(GLFWwindow *window){
	if (windowMode == WindowMode::CommonWin) {
		glfwGetWindowPos(window, &windowedPosX, &windowedPosY);
		glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
	}
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);	// 获取主显示器参数
	glfwSetWindowMonitor(window, NULL, 0, 0, mode->width, mode->height - 1, 0);	// 设置窗口铺满屏幕
	glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);	// 关闭窗口修饰
	windowMode = WindowMode::Frameless;
	Camera::firstMouse = true;
	glfwSetCursorPos(window, mode->width / 2.0f, (mode->height - 1) / 2.0f);	// 重置光标位置
}

void setMSAA(FBO& multiple, FBO& intermediate, int multiNum) {
	// 清理
	cleanupFBO(multiple);
	cleanupFBO(intermediate);
	if (multiNum == 0) {	// 关闭 MSAA
		// 创建帧缓冲
		glGenFramebuffers(1, &multiple.framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, multiple.framebuffer);
		glGenTextures(1, &multiple.textureColorbuffer);
		glBindTexture(GL_TEXTURE_2D, multiple.textureColorbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, multiple.textureColorbuffer, 0);
		glGenRenderbuffers(1, &multiple.rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, multiple.rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, multiple.rbo);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER: Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return; // 跳过后续设置 intermediate
	}
	// 创建多重采样帧缓冲对象
	glGenFramebuffers(1, &multiple.framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, multiple.framebuffer);
	// 创建多重采样纹理附件（multiNum倍 MSAA)
	glGenTextures(1, &multiple.textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multiple.textureColorbuffer);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multiNum, GL_RGB, screenWidth, screenHeight, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, multiple.textureColorbuffer, 0);
	// 创建多重采样渲染缓冲对象（multiNum倍 MSAA)
	glGenRenderbuffers(1, &multiple.rbo); 
	glBindRenderbuffer(GL_RENDERBUFFER, multiple.rbo); 
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, multiNum, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, multiple.rbo);
	// 检查帧缓冲对象是否完整
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)	
		std::cout << "ERROR::FRAMEBUFFER: Mulit Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// 创建中间帧缓冲对象
	glGenFramebuffers(1, &intermediate.framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, intermediate.framebuffer);
	glGenTextures(1, &intermediate.textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, intermediate.textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, intermediate.textureColorbuffer, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER: Intermediate framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cleanupFBO(FBO& fbo) {
    if (fbo.rbo) glDeleteRenderbuffers(1, &fbo.rbo);
    if (fbo.textureColorbuffer) glDeleteTextures(1, &fbo.textureColorbuffer);
    if (fbo.framebuffer) glDeleteFramebuffers(1, &fbo.framebuffer);
    fbo.rbo = 0;
	fbo.textureColorbuffer = 0;
	fbo.framebuffer = 0;
}

float sRGB2Linear(float u) {	// sRGB 色彩标准 IEC 61966-2-1 为非线性编码伽马，其优化了暗部编码，不可简单使用2.2次幂进行还原
    if (u <= 0.04045f) {
        return u / 12.92f;
    } 
	else {
        return powf((u + 0.055f) / 1.055f, 2.4f);
    }
}
void sRGB2Linear(float* colors, int count) {
    for (int i = 0; i < count; i++) {
        colors[i] = sRGB2Linear(colors[i]);	// 遍历数组进行转换
    }
}