#include "shader.h"

// 构造函数读取着色器源码
Shader::Shader(const char* vertexPath, const char* fragmentPath) {
	std::string vertexCode, fragmentCode;	// 源码存储字符串
	std::ifstream vShaderFile, fShaderFile;	// 源码储存文件
	// 设置文件异常掩码
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	// 异常控制
	try {
		// 打开文件
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// 读取缓冲数据
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// 关闭文件
		vShaderFile.close();
		fShaderFile.close();
		// 储存至字符串
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch(std::ifstream::failure error) {
		std::cout << "Not succesfully read shader files" << std::endl;
	}
	// 转换为C风格字符串
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str(); 

	// 编译
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, NULL);
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
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success); 
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infolog);
        std::cout << "Fragment Shader Error!\n"  << infolog << std::endl;
    }

    // 创建着色器程序
    ID = glCreateProgram();	// 获取 ID
    glAttachShader (ID, vertexShader);
    glAttachShader (ID, fragmentShader);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(ID, 512, NULL, infolog);
        std::cout << "Link Shaders Error!\n"  << infolog << std::endl;
    }
	// 释放
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::use() {
	glUseProgram(ID);
}

void Shader::set1Bool(const std::string &name, bool value) const {
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void Shader::set1Int(const std::string &name, int value) const {
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::set1Float(const std::string &name, float value) const {
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::set3Floatv(const std::string &name, float *value) const {
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, value);
}