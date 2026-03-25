#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
	unsigned int ID;

	Shader(const char* vertexPath, const char* fragmentPath);
	
	void use();

	// uniform 工具函数
	void set1Bool(const std::string &name, bool value) const;		// 布尔值
	void set1Int(const std::string &name, int value) const;			// 整型
	void set1Float(const std::string &name, float value) const;		// 浮点数	
	void set3Floatv(const std::string &name, float *value) const;	// 三维浮点数向量	
	void set4Mat(const std::string &name, glm::mat4 value) const;	// 4 * 4 浮点数矩阵	
};

#endif