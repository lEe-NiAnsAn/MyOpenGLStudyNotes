#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
	unsigned int ID;

	Shader(const char* vertexPath, const char* fragmentPath);
	void use();

	// uniform 工具函数
	void set1Bool(const std::string &name, bool value) const;						// 布尔值
	void set1Int(const std::string &name, int value) const;							// 整型
	void set1Float(const std::string &name, float value) const;						// 浮点数	
	void set3Floatv(const std::string &name, float *value) const;					// 三维浮点数向量
	void setVec2(const std::string &name, const glm::vec2 &value) const;			// vec2 容器
	void setVec2(const std::string &name, float x, float y) const;					// vec2 数据
	void setVec3(const std::string &name, const glm::vec3 &value) const;			// vec3 容器
    void setVec3(const std::string &name, float x, float y, float z) const;			// vec3 数据
    void setVec4(const std::string &name, const glm::vec4 &value) const;			// vec4 容器
    void setVec4(const std::string &name, float x, float y, float z, float w) const;// vec4 数据
	void setMat4(const std::string &name, glm::mat4 value) const;					// 4 * 4 浮点数矩阵	
};

#endif