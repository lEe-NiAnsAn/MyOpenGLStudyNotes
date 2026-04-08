/*
深度测试：
一、概念
	1. 深度：物体的每个片段中均储存深度信息，即每一个像素均有一个深度值，决定该像素是否应被显示。
	2. 深度缓冲：拥有与颜色缓冲相同的宽度和高度，精度通常为24位，由窗口系统自动创建，储存了综合了各个片段的所有深度信息。
    3. 深度值：通常以24位 float 的形式储存，为[0.0,1.0]之间的归一化值，本质为摄像机坐标系中的z值。
	4. 深度测试：对一个片段的深度值与当前已写入缓冲的深度内容进行对比。
				当测试刚开始时，缓冲内的深度值均为1.0（远平面端），首个被比较的片段深度值一般直接写入缓冲。
            	若此测试通过，深度缓冲将会更新；反之，被测片段将会被丢弃。（即靠前的像素保留，被遮挡的丢弃）
				物体中后续参与测试的片段依次比较相应位置像素的深度值，直至将所有物体的所有片段测试完毕。
				所有测试完毕后，深度缓冲将在画面帧中控制物体显示何处像素，并通过【glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);】同时清除该帧的深度缓冲与颜色缓冲。
	5. 深度掩码（glDepthMask）：控制深度缓冲是否写入的标志，分别为【GL_TRUE】与【GL_FALSE】
	6. 深度测试函数：修改深度测试中使用的比较运算符，控制何时通过或丢弃片段，何时更新深度缓冲，通常包括八种比较方式。
	7. 深度值精度：实际中通常使用非线性深度缓冲，在 z 值较小时提供较高精度，而在 z 值较远时提供较少精度。
	8. 深度冲突：当两个平面的 z 值相同或极其相近（精度不足）时出现彼此争夺更靠前位置的情形，通常仅可避免而非消除。
二、时机
	深度缓冲在片段着色器运行之后（以及模板测试运行之后）于屏幕空间中运行。
	目前大部分的 GPU 都提供提前深度测试的硬件特性，允许深度测试在片段着色器之前运行，可提前丢弃永远不可见的片段，避免不必要的着色开销；
	当使用提前深度测试时，片段着色器不可修改深度值。
*/

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "shader.h"
#include "mesh.h"
#include "model.h"

// 窗口随视口尺寸变化
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
// 鼠标指针事件
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
// 鼠标滚轮事件
void scroll_callback(GLFWwindow* window, double xoffsetIn, double yoffsetIn);
// 按键事件
void processInput(GLFWwindow *window);

static bool Border = true;  // 是否显示边框
float delta = 0.0f;         // 帧间时差
float lastFrame = 0.0f;     // 末帧时间
// 摄像机对象创建
Camera myCamera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));  
int screenWidth = 800;
int screenHeight = 800;	// 设置显示宽高
float Camera::m_cursorX = screenWidth / 2.0f;
float Camera::m_cursorY = screenHeight / 2.0f;	// 鼠标指针位置窗口居中
bool Camera::firstMouse = true;

int main() {
	// 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // 创建窗口
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Learning Chapter12", NULL, NULL);
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
	// 同步视口变化窗口尺寸
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
    
	// 启用深度测试
	glEnable(GL_DEPTH_TEST);	
    // 启用模板测试
    // 比较各个片段被赋予的八位模板值，控制特定像素的可见性
    // 例如：设定一个模板值为1的区域，并控制模板值不等于1的像素不通过测试，则只有该区域内（模板值为1）的像素才会被渲染
    glEnable(GL_STENCIL_TEST);
	
    Shader myShader("src/shaders/shader.vert", "src/shaders/shader.frag");  // 使用着色器类进行着色器导入
    Shader borderShader("src/shaders/shader.vert", "src/shaders/border_shader.frag");
    Model myModel("src/models/test.obj");           // 使用模型类进行模型导入
    
    while (!glfwWindowShouldClose(window)){
        processInput(window);
        
        // 背景重绘
		glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);	// 清除屏幕缓冲、深度缓冲与模板缓冲
        
        // 光源设置
        myShader.use();
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);	    // 设置光源颜色
        glm::vec3 direction =myCamera.m_front;      // 设置光照方向同摄像机所摄方向
        glm::vec3 lightPos = myCamera.m_position;	// 设置光源位置同摄像机位置
        myShader.setVec3("light.direction", direction);
        myShader.setVec3("light.position", lightPos);
        myShader.set1Float("light.innerCutOff", cosf(glm::radians(11.0f)));	// 传入聚光内切光角余弦值
        myShader.set1Float("light.outerCutOff", cosf(glm::radians(13.0f)));	// 传入聚光外切光角余弦值
        myShader.setVec3("light.diffuse", lightColor);						// 传入漫反射光强分量
        myShader.setVec3("light.specular", glm::vec3(1.5f) * lightColor);	// 传入镜面反射光强分量
        myShader.setVec3("light.ambient", glm::vec3(0.5f) * lightColor);	// 传入环境光强分量
        myShader.set1Float("material.shininess", 32.0f);					// 传入镜面高光散射度
        float linear{0.2f}, quadratic{0.022f};                           	// 设置光照衰减函数一次项、二次项
        myShader.set1Float("light.linear", linear);
        myShader.set1Float("light.quadratic", quadratic);
        myShader.set1Float("light.constant", 1.0f);
        
        // 透视裁剪
        glm::mat4 projection = glm::perspective(glm::radians(myCamera.m_zoom), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		// 观察视角
        glm::mat4 view = myCamera.GetViewMatrix();
        myShader.setMat4("projection", projection);
        myShader.setMat4("view", view);
		// 位移缩放
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        myShader.setMat4("model", model);
		
        // 设置模板测试函数：定义如何进行测试（比较条件、参考值、掩码）
        glStencilFunc(GL_ALWAYS, 1, 0xFF);  // 模板测试在与参考值比较后无论条件始终通过，参考值设定为1，掩码表示使用全八位模板值
        // 设置模板操作函数：指定模板测试后进行的操作（本处为清空模板缓冲后始终将区域内的模板值赋值为1）
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  // 模板测试失败时保持模板缓冲不变，模板测试成功但深度测试失败时依旧不变，二者均成功时替换当前模板缓冲为设置的参考值
        // 启用模板缓冲写入：定义写入掩码
        glStencilMask(0xFF);    // 将参考值全八位写入模板值
        myModel.Draw(myShader); // 绘制物体主体
        
        if (Border) {        
            // 设置另一模板区域绘制物体边框
            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);    // 模板状态机更新至模板值不等于1的区域
            glStencilMask(0x00);    // 只读模板缓冲（掩码全0）
            glDisable(GL_DEPTH_TEST);   // 关闭深度测试
            // 设置轮廓的空间变换矩阵
            borderShader.use();
            float scale = 1.01f; // 轮廓大小
            borderShader.setMat4("projection", projection);
            borderShader.setMat4("view", view);
            glm::mat4 border_model = glm::scale(model, glm::vec3(scale));   // 放大
            borderShader.setMat4("model", border_model);
            myModel.Draw(borderShader);
        }        
        // 恢复模板缓冲状态
        glStencilMask(0xFF);    // 模板缓冲全开掩码
        glStencilFunc(GL_ALWAYS, 0, 0xFF);  // 赋值所有模板值为0
        
        glEnable(GL_DEPTH_TEST);    // 恢复深度测试状态为开启
        
		glfwSwapBuffers(window);
        glfwPollEvents();
	}

    glfwTerminate();
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) { // 传入窗口句柄与尺寸（回调函数）
    glViewport(0, 0, width, height);
    screenWidth = width;
    screenHeight = height;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) { // 传入窗口句柄与鼠标坐标（回调函数）
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    myCamera.ProcessMouseMovement(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffsetIn, double yoffsetIn) {  // 传入窗口句柄与滚轮运动参数（回调函数）
    float yoffset = static_cast<float>(yoffsetIn);
    myCamera.ProcessMouseScroll(yoffset);
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
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            direction = Camera::Camera_Movement::REZOOM;
            std::cout << "The FOV has been reset to default value." << std::endl;
        }    
    }
	lastRState = currentRState;
    myCamera.ProcessKeyboard(direction, delta);
    // 按 B 切换边框显示模式
    static bool lastBState = false; // 最后 B 键状态
    bool currentBState = glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS;
    if (currentBState && !lastBState) {
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
            if (Border) Border = false;
            else Border = true;
            std::cout << "The border mode has been changed." << std::endl;
        }    
    }
	lastBState = currentBState;
    // 按下 Esc 退出窗口
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }   
}