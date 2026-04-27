#include "shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath) {
	std::string vertexCode, geometryCode, fragmentCode;
	std::ifstream vShaderFile, gShaderFile, fShaderFile;
	// 设置文件异常掩码
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		// 打开文件
		vShaderFile.open(vertexPath);
		if (geometryPath != NULL) gShaderFile.open(geometryPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, gShaderStream, fShaderStream;
		// 读取缓冲数据
		vShaderStream << vShaderFile.rdbuf();
		if (geometryPath != NULL) gShaderStream << gShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// 关闭文件
		vShaderFile.close();
		if (geometryPath != NULL) gShaderFile.close();
		fShaderFile.close();
		// 储存至字符串
		vertexCode = vShaderStream.str();
		geometryCode = gShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch(std::ifstream::failure error) {
		std::cout << "Not succesfully read shader files" << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* gShaderCode = geometryCode.c_str(); 
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
	unsigned int geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
	if (geometryPath != NULL) {	
		glShaderSource(geometryShader, 1, &gShaderCode, NULL);
		glCompileShader(geometryShader);
		glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(geometryShader, 512, NULL, infolog);
			std::cout << "Geometry Shader Error!\n"  << infolog << std::endl;
		}
	}    
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success); 
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infolog);
        std::cout << "Fragment Shader Error!\n"  << infolog << std::endl;
    }

    // 创建着色器程序
    ID = glCreateProgram();
    glAttachShader (ID, vertexShader);
    if (geometryPath != NULL) glAttachShader (ID, geometryShader);
    glAttachShader (ID, fragmentShader);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(ID, 512, NULL, infolog);
        std::cout << "Link Shaders Error!\n"  << infolog << std::endl;
    }
	// 释放
    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
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
void Shader::setVec2(const std::string &name, const glm::vec2 &value) const { 
	glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}
void Shader::setVec2(const std::string &name, float x, float y) const { 
	glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); 
}
void Shader::setVec3(const std::string &name, const glm::vec3 &value) const { 
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}
void Shader::setVec3(const std::string &name, float x, float y, float z) const { 
	glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); 
}
void Shader::setVec4(const std::string &name, const glm::vec4 &value) const { 
	glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}
void Shader::setVec4(const std::string &name, float x, float y, float z, float w) const { 
	glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); 
}
void Shader::setMat4(const std::string &name, glm::mat4 value) const {	
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}