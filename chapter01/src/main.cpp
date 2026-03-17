#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 注册“改变视口维度”回调函数  
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);    // 防止窗口内容变化后显示不完整或变形
}

// 注册“输入反应”回调函数
void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {	// 若按下 Esc 键
		glfwSetWindowShouldClose(window, true);	// 窗口退出
	}
}

int main() {
    glfwInit(); // 初始化 GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  // 配置 GLFW 主版本号
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);  // 配置 GLFW 次版本号
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 告知 GLFW 使用模式为核心模式

    // 创建窗口（宽，高，名称，显示器，共享资源的上下文窗口）
    GLFWwindow* window = glfwCreateWindow(800, 600, "Learning Chapter01", NULL, NULL);
    if (window == NULL) {   // 创建失败
        std::cout << "创建 GLFW 窗口失败！" << std::endl;
        glfwTerminate();    // 清理 GLFW 所占用的资源	
        return -1;
    }
    glfwMakeContextCurrent(window); // 将指定上下文设置为当前线程的当前上下文

    // 初始化窗口（GLAD所定义的类型的“获取函数地址”函数的指针）
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {  // 初始化失败
        std::cout << "无法初始化 GLFW 窗口！" << std::endl;
        return -1;
    }

    // 设置视口尺寸(左下角X坐标，左下角Y坐标，宽，高)
    glViewport(0, 0, 800, 600); // OpenGL中的位置坐标需转换（映射）为屏幕坐标（故该函数设置的维度可比 GLFW 小）
    // "设置窗口帧缓冲区大小变化"回调函数（窗口指针，改变视口维度指针）
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  // 每次窗口调整大小时调用“改变视口维度”

	float color_r = 0.0f;
	float color_b = 0.0f;
	float color_g = 0.0f;
	float angle = 0.0f;                
    const float angle_step = 0.00003f;
    const float phase_offset = 2 * M_PI / 3;	// 三原色相位差

    // 渲染循环
    while (!glfwWindowShouldClose(window)){ // 窗口未被要求退出
		processInput(window);	// 检测输入

		// 背景颜色持续变化
        color_r = (sin(3*angle) + 1.0f) / 2.0f;	// (sin3x + 1) ÷ 2
        color_g = (sin(2*angle + phase_offset) + 1.0f) / 3.0f;	// (sin(2x+2π/3) + 1) ÷ 3
        color_b = (sin(angle + 2 * phase_offset) + 1.0f) / 2.0f;	// (sin(x+4π/3) + 1) ÷ 2
		angle += angle_step;
        if (angle > 2 * M_PI) {
            angle -= 2 * M_PI;	// 保证不超过2π
        }
        angle += angle_step;
        if (angle > 2 * M_PI) {
            angle -= 2 * M_PI;	// 保证不超过2π
        }
		glClearColor(color_r, color_g, color_b, 1.0f);	// 设置当前“后缓冲”背景颜色
        glClear(GL_COLOR_BUFFER_BIT);	// 清除当前“前缓冲”中的背景颜色
        
		glfwPollEvents();	 // 检查并调用事件
        glfwSwapBuffers(window);	// 交换颜色缓冲（即在此“前缓冲”显示“后缓冲”的画面）
    }

    glfwTerminate();    // 释放资源
    return 0;
}