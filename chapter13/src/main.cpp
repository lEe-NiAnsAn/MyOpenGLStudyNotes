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

float delta = 0.0f;     // 帧间时差
float lastFrame = 0.0f; // 末帧时间
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
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Learning Chapter13", NULL, NULL);
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
	// 启用深度测试
	glEnable(GL_DEPTH_TEST);	
	// 检测鼠标移动事件并调用回调函数
	glfwSetCursorPosCallback(window, mouse_callback);
	// 检测键盘输入时间并调用回调函数
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// 检测鼠标滚轮事件并调用回调函数
	glfwSetScrollCallback(window, scroll_callback);
    
    // 启用混合
    glEnable(GL_BLEND);
    // 两种设置混合操作函数（设置源像素和目标像素的混合计算占比因子）
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // 源因子 = 源alpha值；目标因子 = 1 - 源alpha值
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO); // RGB因子设置同上，
                                                                                // 再将源alpha通道因子设置为1，源alpha值完全覆盖目标alpha值；
                                                                                // 后将目标alpha通道因子设置为0，目标alpha值不对最终的alpha值产生影响。
    // 默认混合运算函数（设置运算符，默认相加）
    // glBlendEquation(GL_FUNC_ADD);
    
    Shader myShader("src/shaders/shader.vert", "src/shaders/shader.frag");  // 使用着色器类进行着色器导入
    myShader.use();
    Model myModel("src/models/test.obj");           // 使用模型类进行模型导入
    
    // 位移缩放
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    myShader.setMat4("model", model);
    glm::mat4 inverseModel = glm::inverse(model);   // 外置位移矩阵以减轻计算逆矩阵的压力
    
    while (!glfwWindowShouldClose(window)){
		processInput(window);

        // 背景重绘
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// 清除屏幕缓冲与深度缓冲

        // 光源设置
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);	    // 设置光源颜色
        glm::vec3 direction =myCamera.m_front;      // 设置光照方向同摄像机所摄方向
        glm::vec3 lightPos = myCamera.m_position;	// 设置光源位置同摄像机位置
        myShader.setVec3("light.direction", direction);
        myShader.setVec3("light.position", lightPos);
        myShader.set1Float("light.innerCutOff", cosf(glm::radians(11.0f)));	// 传入聚光内切光角余弦值
        myShader.set1Float("light.outerCutOff", cosf(glm::radians(13.0f)));	// 传入聚光外切光角余弦值
        myShader.setVec3("light.diffuse", lightColor);                      // 传入漫反射光强分量
        myShader.setVec3("light.specular", lightColor);	                    // 传入镜面反射光强分量
        myShader.setVec3("light.ambient", glm::vec3(0.6f) * lightColor);    // 传入环境光强分量
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
        
        glm::vec3 viewPos = glm::vec3(inverseModel * glm::vec4(myCamera.m_position, 1.0f)); // 计算位于模型局部的摄像机坐标
        myModel.Draw(myShader, viewPos);
        
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
    // 按下 Esc 退出窗口
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }   
}