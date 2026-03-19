#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
	unsigned int ID;	// 程序ID

	Shader(const char* vertexPath, const char* fragmentPath);	// 构造函数
	
	void use();	// 使用

	// uniform 不同类型的属性
	void set1Bool(const std::string &name, bool value) const;
	void set1Int(const std::string &name, int value) const;
	void set1Float(const std::string &name, float value) const;
	void set3Floatv(const std::string &name, float *value) const;
};

#endif