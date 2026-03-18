#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// 编写着色器
/* 使用着色器语言 GLSL 编写：
 *
 *  // 顶点着色器：
 *  #version 330 core                       // 版本声明：3.3GL，核心模式
 *  layout (location = 0) in vec3 aPos;     // 使用 in 关键字在顶点着色器中声明所有的输入顶点属性（此处仅需“坐标”一项）
 *                                          // 声明向量类型 vec3 的输入变量 aPos ，设定输入变量的位置值 location
 *  void main() {                           // vec4 的前三个分量为对应坐标值，第四个分量为透视除数值
 *      gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
 *  }
 * 
 * // 片段着色器：
 * #version 330 core
 * out vec4 FragColor;                       // 使用 out 关键字声明输出变量
 * 
 * void main() {                            // RGBA 四维颜色向量
 *  FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
 * }
 */
// 硬编码着色器源代码
// 顶点着色器：
const char *vertexShaderSource = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main() {\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
// 片段着色器：
const char *fragmentShaderSource = 
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "   FragColor = vec4(0.1f, 0.2f, 0.7f, 1.0f);\n"
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
    
    // 运行时编译着色器
    unsigned int vertexShader;  // 通过无符号整型 ID 引用顶点着色器对象
    vertexShader = glCreateShader(GL_VERTEX_SHADER);    // 创建着色器对象（着色器类型）
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // 附加顶点着色器源码（着色器对象，字符串数量，源码字符数组指针，源码尾部字符）
    glCompileShader(vertexShader);  // 编译顶点着色器
    int vertex_success;    // 顶点着色器编译状态标识
    char vertex_infolog [512];  // 顶点着色器编译信息容器
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertex_success);   // 获取编译状态
    if (!vertex_success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, vertex_infolog);    // 获取编译信息
        std::cout << "Vertex Shader Error!\n"  << vertex_infolog << std::endl;
    }
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    int fragment_success;    
    char fragment_infolog [512];  
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragment_success); 
    if (!fragment_success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, fragment_infolog);
        std::cout << "Fragment Shader Error!\n"  << fragment_infolog << std::endl;
    }

    // 创建着色器程序
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();  // 创建一个程序对象并返回其ID引用
    glAttachShader (shaderProgram, vertexShader);   // 附加顶点着色器
    glAttachShader (shaderProgram, fragmentShader); // 附加片段着色器
    glLinkProgram(shaderProgram);   // 链接所有着色器
    int link_success;
    char link_infolog [512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &link_success);
    if(!link_success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, link_infolog);
        std::cout << "Link Shaders Error!\n"  << link_infolog << std::endl;
    }
    glDeleteShader(vertexShader);   // 释放顶点着色器对象
    glDeleteShader(fragmentShader); // 释放片段着色器对象


    // 设置顶点数组（采用标准化设备坐标来表示,在视口改变尺寸时可能会变形）
    float vertices01[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };
    unsigned int VAO01;
    unsigned int VBO01; // 顶点缓冲对象 ID 为无符号整型，为“代号”
    glGenVertexArrays(1, &VAO01);   
    glGenBuffers(1, &VBO01);    // 生成带缓冲ID的顶点缓冲对象（只需知道其 ID 即可，VBO内部结构无须在意）
    glBindVertexArray(VAO01);   // 绑定至当前缓冲数组（进入“状态机”，储存接下来的设置）
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO01);   // 绑定至相应类型（可同时绑定多个不同类型的缓冲）
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices01), vertices01, GL_STATIC_DRAW);  // 将顶点数据复制至当前绑定的VBO（ GL_STATIC_DRAW 标志为告知计算机该数据几乎不会改变）
    
    // 定义顶点属性的数据格式和布局（输入变量位置值，单个“属性”的组成大小，“属性”组成值的类型，是否标准化，步长，起始位置偏移量）
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);   // 通过输入变量位置值定位VBO启用顶点属性
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);   // 解绑当前 VBO
    glBindVertexArray(0);   // 解绑当前 VAO

    // 元素缓冲对象
    float vertices02[] = {    // 通过两个三角形绘制正方形
        -0.2f, 0.7f, 0.0f, // 右上角
        -0.2f, 0.2f, 0.0f, // 右下角（通过EBO这个缓冲区避免了顶点的重复）
        -0.7f, 0.7f, 0.0f, // 左下角
        -0.7f, 0.2f, 0.0f  // 左上角（避免重复）
    };
    unsigned int indices[] = {  // 通过索引确定各个三角的顶点值
        0, 1, 2, 
        1, 2, 3
    };
    unsigned int VAO02, EBO01, VBO02;
    glGenVertexArrays(1, &VAO02);
    glGenBuffers(1, &EBO01);    // 生成元素缓冲对象
    glGenBuffers(1, &VBO02);
    glBindVertexArray(VAO02);
    glBindBuffer(GL_ARRAY_BUFFER, VBO02);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices02), vertices02, GL_STATIC_DRAW);  // 复制顶点数据至 VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO01);   // 绑定
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // 复制
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);   // 设置顶点属性
    glEnableVertexAttribArray(0);   // 输入变量位置值定位
    // 不可提前解绑 VAO ：当目标为 GL_ELEMENT_ARRAY_BUFFER 时，VAO 会储存 glBindBuffer 的函数调用，其中便储存了解绑调用（即将 EBO 的绑定记录为“解绑”）
    glBindBuffer(GL_ARRAY_BUFFER, 0);   // 解绑 VBO
    glBindVertexArray(0);   // 解绑 VAO

    while (!glfwWindowShouldClose(window)){
		processInput(window);

		glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 绘制图元
        glUseProgram(shaderProgram);    // 激活着色器程序
        glBindVertexArray(VAO01);   // 使用该 VAO 配置进行渲染
        glDrawArrays(GL_TRIANGLES, 0, 3);   // 绘制顶点数组内容（图元类型，起始索引，绘制顶点数）
		glBindVertexArray(VAO02);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	// 设置光栅化模式（指定作用面，指定模式）
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);    // 绘制缓冲元素（图元类型，顶点个数，索引类型，偏移量）
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// 设置回初始值

		glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO01);
    glDeleteBuffers(1, &VBO01);
	glDeleteBuffers(1, &VAO02);
	glDeleteBuffers(1, &EBO01);
    glDeleteProgram(shaderProgram);

    
    glfwTerminate();
    return 0;
}