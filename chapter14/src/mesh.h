#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>

#include <string>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"

struct Vertex {		// 顶点属性
    glm::vec3 Position;		// 位置向量
    glm::vec3 Normal;		// 法线向量
    glm::vec2 TexCoords;	// 纹理坐标
};

struct Texture {	// 纹理属性
    unsigned int id;		// 纹理 ID
    std::string type;		// 纹理类型
	std::string path;		// 纹理路径	
};

class Mesh {
public:
	std::vector<Vertex> vertices;		// 顶点容器
	std::vector<unsigned int> indices;	// 元素索引
	std::vector<Texture> textures;		// 纹理容器
	unsigned int VAO;					// 顶点数组对象 ID
	// 构造函数
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	void Draw(Shader &shader);			// 绘制纹理

private:
	unsigned int  VBO, EBO;				// 顶点缓冲对象与元素缓冲对象 ID
	void setupMesh();					// 设置顶点
};

#endif