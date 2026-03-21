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

	void set1Bool(const std::string &name, bool value) const;
	void set1Int(const std::string &name, int value) const;
	void set1Float(const std::string &name, float value) const;
	void set3Floatv(const std::string &name, float *value) const;
	void set4Mat(const std::string &name, glm::mat4 value) const;
};

#endif