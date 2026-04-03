/*
项目笔记

一、概述渲染流程：
	1. 初始化 GLFW，形成窗口渲染的主体基础框架，具体实现回调函数；
	2. 创建摄像机类与着色器类对象，定义空间转换四步骤矩阵：[局部] --位移--> [世界] --视角--> [观察] --剪切--> [屏幕]；
	3. 编写着色器有关空间转换的部分；
	4. 创建手电型光源，编写传递光源参数设置的着色器与主程序代码：环境反射、漫反射、镜面反射；
	5. 创建模型类对象，导入自定模型类型数据后绘制模型。
二、主要类：模型类
	1. 首先创建网格类，定义模型内部数据结构，并实现模型数据参数传入 VAO、VBO、EBO 中以便调用相关渲染函数；
	2. 创建模型类，实现外部类型的模型文件数据的导入，生成先前自定义的网格类对象为模型数据，包括顶点位置向量数据、法向量数据、纹理坐标数据、纹理图像数据；
	3. 根据自定的模型数据结构，在模型类的构造函数中调用读取数据方法，并传入网格类的物体绘制方法中实现模型的绘制。
三、详细步骤：
	1. 创建窗口 --> 基础框架
	（1）确定 GLFW 版本
	（2）创建窗口对象，设置关联上下文
	（3）初始化窗口、视口，调整相关显示设置
	（4）调用窗口尺寸变化、按键响应、滚轮响应、鼠标响应回调函数
	（5）进入绘制主循环，设置背景与深度，交换前后缓冲
	（6）清理资源退出绘制
	2. 摄像机类 --> 视角控制
	（1）设置参数：视角变换与移动、摄像机三轴与坐标
	（2）构造函数：设定摄像机初始位置
	（3）计算视角变换矩阵 LookAt
	（4）检测键盘输入移动摄像机坐标
	（5）检测鼠标滑动改变摄像机视角
	（6）检测滚轮滑动改变 FOV
	（7）工具函数：计算摄像机视角方向
	3. 着色器类 --> 编译着色器文件
	（1）设置参数与成员数据：着色器程序 ID、不同类型 uniform 参数传入方法
	（2）构造函数：读取着色器文件数据至字符串；编译着色器；创建着色器程序并链接各阶段着色器
	（3）着色器启动函数 API
	4. 着色器 --> 渲染与光照处理
	（1）顶点着色器：接收后传出顶点位置、法线向量、纹理坐标；获取空间变换矩阵 uniform 变量
	（2）片段着色器：定义光源结构体；
				   接收顶点位置、法线向量、纹理坐标；
				   获取纹理图像数据与光源数据；
				   计算环境光照、漫反射光照、镜面反射光照、聚光插值、距离光强衰减系数；
				   合成并传出片段颜色
	5. 网格类 --> 处理模型数据的工具类
	（1）设置网格数据：位置向量、法线向量、纹理坐标
	（2）设置纹理图片属性：ID、类型、路径
	（3）定义模型数据结构：顶点数组（顶点位置、法线向量、纹理坐标）容器、元素索引（EBO）容器、纹理容器、渲染三对象 ID
	（4）构造函数：接受上述数据三容器数据创建包含模型所有网格的对象
	（5）初始化函数：初始化渲染三对象，传递顶点数据容器与元素索引容器数据至相应对象，并定位传入着色器
	（6）渲染函数：分类激活不同类型纹理图像后传入着色器，并调用渲染函数
	6. 模型类 --> 导入模型文件
	（1）私有数据：网格容器、已加载模型记录容器、模型目录
	（2）构造函数：传入模型路径，调用导入模型函数
	（3）导入模型函数：使用导入器读取模型文件，递归调用节点处理函数读取根节点及其所有子节点数据；设置模型目录
	（4）节点处理函数：递归调用网格处理函数遍历所有网格
	（5）网格处理函数：分阶段读取由导入器导入的数据至顶点位置向量容器、法线向量容器、纹理坐标容器、元素索引容器、纹理材质容器
	（6）纹理加载工具函数：通过类型读取纹理至已加载模型记录容器，并通过纹理读取工具方法获取分配的纹理 ID
	（7）纹理读取工具方法：通过纹理文件名与纹理路径调用纹理生成与绑定函数初始化载入的纹理图像，并设置纹理选项
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
	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Learning Chapter11", NULL, NULL);
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

    Shader myShader("src/shaders/shader.vert", "src/shaders/shader.frag");  // 使用着色器类进行着色器导入
    myShader.use();
    Model myModel("src/models/test.obj");           // 使用模型类进行模型导入

    while (!glfwWindowShouldClose(window)){
		processInput(window);

        // 背景重绘
		glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// 清除屏幕缓冲与深度缓冲

        // 光源设置
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
		
        myModel.Draw(myShader);

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