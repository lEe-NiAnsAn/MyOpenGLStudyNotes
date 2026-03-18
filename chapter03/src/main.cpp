#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

/*  着色器语言GLSL概要：

1. 基本结构
#version version_number
in type in_variable_name;
in type in_variable_name;

out type out_variable_name;

uniform type uniform_name;

void main() {
  // 处理输入并进行一些图形操作
  ...
  // 输出处理过的结果到输出变量
  out_variable_name = weird_stuff_we_processed;
}

2. 常用a类型
整型int、无符号整型uint、
浮点数float、双精度浮点数double、
布尔值bool、
向量vector、矩阵matrix

3. 向量（“n”代表2、3或4）
vecn		包含 n 个 float 分量的默认向量
bvecn		包含 n 个 bool 分量的向量
ivecn		包含 n 个 int 分量的向量
uvecn		包含 n 个 unsigned int 分量的向量
dvecn		包含 n 个 double 分量的向量
（	可用 .x  .y  .z  .w 获取第1、2、3、4个分量，另外：
	对于颜色可用 .r  .g  .b  .a ，
	对于纹理坐标可用 .s  .t  .p  .q	）

4. 重组（特殊向量分量选择方式）
示例一：
vec2 someVec;
vec4 differentVec = someVec.xyxx;
vec3 anotherVec = differentVec.zyw;
vec4 otherVec = someVec.xxxx + anotherVec.yxzy;
示例二：
vec2 vect = vec2(0.5, 0.7);
vec4 result = vec4(vect, 0.0, 0.0);
vec4 otherResult = vec4(result.xyz, 1.0);

5. 输入与输出
关键字 in			定义接受从上一着色器阶段的何类输入			例：in vec4 vertexColor;
关键字 out			定义发送至下一着色器阶段的何类输出			例：out vec4 fragColor;
关键字 layout		顶点着色器的特殊直接输入（标识）			例：layout (location = 0) in vec3 aPos;
（只有一个输出变量与下一着色器阶段的输入类型与变量名均匹配时才会链接传递）

6. 关键字 uniform（应用程序在 CPU 上传递数据至 GPU 上着色器）
着色器代码：
#version 330 core
out vec4 FragColor;

uniform vec4 ourColor; // 可在任意着色器中定义此【全局】变量，无需通过顶点着色器作为中介

void main() {
    FragColor = ourColor;
}
主程序代码：
float timeValue = glfwGetTime();	// 获取程序运行的当前时间
float greenValue = (sin(timeValue) / 2.0f) + 0.5f;	// 将时间转为正弦波
int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");	// 获取着色器中 ourColor 变量内存位置
glUseProgram(shaderProgram);	// 查询 uniform 地址不要求提前使用着色器程序，但更新前必须先使用
glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);	// 向当前激活的着色器程序的 ourColor 传入 RGBA 值
*/

const char *vertexShaderSource = 
	"#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"	// 位置变量属性位置值为0
	"layout (location = 1) in vec3 aColor;\n"	// 颜色变量属性位置值为1
	"out vec3 myColor;\n"	// 向片段着色器输出一个颜色
	"void main() {\n"
	"	gl_Position = vec4(aPos, 1.0);\n"
	"	myColor = aColor;\n"	// 设置输出颜色为顶点数据中的值
	"}\0";
const char *fragmentShaderSource = 
	"#version 330 core\n"
	"out vec4 FragColor;\n " 
	"in vec3 myColor;\n"	// 接收初始颜色值
	"uniform vec3 time;\n"  // 【全局变量】控制颜色变化
	"void main() {\n"
	"	vec3 tempColor = myColor + time;\n"	// 初始值与变量值相加取浮点数绝对值
	"	tempColor = min(tempColor, vec3(1.0));\n"	// 防止溢出
	"	FragColor = vec4(tempColor, 1.0);\n"	// 无法直接修改由顶点着色器输入的变量属性
	"}\0";

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Learning Chapter02", NULL, NULL);
    if (window == NULL) {
        std::cout << "Create GLFW Window Failed!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Initial GLFW Windows Failed!" << std::endl;
        return -1;
    }

    glViewport(0, 0, 800, 600);

    // 测试可声明的顶点属性上限值（本机为16）
    int nrAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
    std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;

	// 编译着色器源代码
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int success;
    char infolog [512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infolog);
        std::cout << "Vertex Shader Error!\n"  << infolog << std::endl;
    }
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success); 
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infolog);
        std::cout << "Fragment Shader Error!\n"  << infolog << std::endl;
    }

    // 创建着色器程序
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader (shaderProgram, vertexShader);
    glAttachShader (shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infolog);
        std::cout << "Link Shaders Error!\n"  << infolog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

	// 顶点数组
	float vertices[] = {
		// 位置					// 颜色
		 0.5f, -0.5f, 0.0f,		0.8f, 0.0f, 0.0f,	// 右下（红）
		-0.5f, -0.5f, 0.0f,		0.0f, 0.7f, 0.0f,	// 左下（绿）
		 0.0f,  0.5f, 0.0f,		0.0f, 0.0f, 0.9f 	// 顶部（蓝）
	};

	// 生成配置 VAO、VBO
	unsigned int VAO,VBO;
    glGenVertexArrays(1, &VAO);   
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// 步长 6*4 字节（位置属性值与颜色属性值），起始偏移量 0 字节
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);	// 位置值0的位置属性
	// 步长 6*4 字节，起始偏移量 3*4 字节（前 3*4 字节是位置属性值）
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);	// 位置值1的颜色属性
	// 片段着色器的片段插值：
	// 当渲染图形时，光栅化阶段通常会线性插入更多片段，根据其所处相对位置决定片段属性，形成平滑过渡的效果
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window)){
		processInput(window);

		glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

		// 使用着色器
		glUseProgram(shaderProgram);	// 启动
		int vertexColorLocation = glGetUniformLocation(shaderProgram, "time");	// 定位
		float timeValue = glfwGetTime();	// 获取运行时间
		float colorValue[] = {	// 三角函数“潮汐”变化修改值
			fabsf(sinf(1.7 * timeValue)/1.7f),
			fabsf(cosf(1.3 * timeValue)/2.3f),
			fabsf(sinf(0.7 * timeValue)/1.3f)
		};
		glUniform3fv(vertexColorLocation, 1, colorValue);	// 传入
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);	// 绘图

		glfwSwapBuffers(window);
        glfwPollEvents();
    }

	glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    
    glfwTerminate();
    return 0;
}