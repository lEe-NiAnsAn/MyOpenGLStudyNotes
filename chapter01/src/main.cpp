#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// 注册“改变视口维度”回调函数  
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);    // 防止窗口内容变化后显示不完整或变形
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

    // 渲染循环
    while (!glfwWindowShouldClose(window)){ // 窗口未被要求退出
        glfwSwapBuffers(window);    // 检查是否触发事件
        glfwPollEvents();   // 交换颜色缓冲（即在此“后缓冲”显示“前缓冲”的画面）
    }

    glfwTerminate();    // 释放资源
    return 0;
}