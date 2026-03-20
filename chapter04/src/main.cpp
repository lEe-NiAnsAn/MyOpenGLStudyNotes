#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION	// 预处理器会修改头文件，让其只包含相关的函数定义源码
#include "stb/stb_image.h"

#include "shader.h"

// 窗口随视口尺寸变化
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}
// 按下 Esc 退出
void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}


int main() {
    // 初始化
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(800, 800, "Learning Chapter04", NULL, NULL);
    if (window == NULL) {
        std::cout << "Create GLFW Window Failed!" << std::endl;
        glfwTerminate();
        return -1;
    }
    // 设置上下文
    glfwMakeContextCurrent(window);
    // 变化窗口尺寸
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // 窗口初始化
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Initial GLFW Windows Failed!" << std::endl;
        return -1;
    }
    // 视口尺寸
    glViewport(0, 0, 800, 800);

    // 编译、创建着色器
    Shader myShader("src/shaders/shader.vert","src/shaders/shader.frag");

    // 创建纹理
    unsigned int texture;	// 引用 ID
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);	// 绑定该2D纹理，进入对应状态机
    // 设置各项参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// S 轴方向设置默认重复纹理
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);	// T 轴方向设置默认重复纹理
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINE);	// 缩小操作设置线性过滤
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINE);	// 放大操作设置线性过滤
    // 载入图像
    int width, height, nrChannels;	// 宽，高，颜色通道数
    stbi_set_flip_vertically_on_load(true); // OpenGL 中 y 轴 0 坐标在图片底部，而 stbi 中 y 轴0坐标在顶部，需要在载入图片前调用该函数翻转
    unsigned char *data = stbi_load("src/textures/laughlikes.jpg", &width, &height, &nrChannels, 0);	// 参数 0 表示保持原通道数
    if(data) {
        // 生成纹理（纹理目标，多级渐远纹理级别，储存格式，纹理宽，纹理高，边框必需为0，源图格式，源图数据类型，图像数据）
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);	// 附着该纹理至绑定值
        glGenerateMipmap(GL_TEXTURE_2D);	// 为当前绑定的纹理自动生成多级渐远纹理
    }
    else {
        std::cout << "Loading texture error!" << std::endl;
    }
    stbi_image_free(data);	// 释放图像内存
    
    // 元素缓冲对象
    float vertices[] = { 
        // ---- 位置 ----		---- 颜色 ----		- 纹理坐标 -
         0.5f, -0.5f, 0.0f,		0.7f, 0.4f, 0.0f,	1.0f, 0.0f,	// 右下
         0.5f,  0.5f, 0.0f,		0.7f, 0.0f, 0.0f,	1.0f, 1.0f,	// 右上
        -0.5f, -0.5f, 0.0f,		0.0f, 0.7f, 0.0f,	0.0f, 0.0f,	// 左下
        -0.5f,  0.5f, 0.0f,		0.0f, 0.0f, 0.8f,	0.0f, 1.0f 	// 左上    
    };
    
    unsigned int indices[] = {	// 步长为 8*4 字节
        0, 1, 2, 
        1, 2, 3
    };
	// 生成配置 VAO、EBO、VBO
	unsigned int VAO, EBO, VBO;
    glGenVertexArrays(1, &VAO);   
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0));	// 步长为 8*4 字节 
    glEnableVertexAttribArray(0);	// 位置
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);	// 颜色
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);	// 纹理坐标
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    

    while (!glfwWindowShouldClose(window)){
        processInput(window);
        // 背景重绘
		glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // 加载着色器绘制
        myShader.use();
        float timeValue = glfwGetTime();
        float colorValue[] = {timeValue, timeValue, timeValue};
        myShader.set3Floatv("time", colorValue);
        glBindTexture(GL_TEXTURE_2D, texture);  // 绑定纹理
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // 释放缓冲与着色器
	glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(myShader.ID);

    glfwTerminate();
    return 0;
}