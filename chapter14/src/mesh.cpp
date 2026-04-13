#include "mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    setupMesh();
}

void Mesh::Draw(Shader &shader) {
	unsigned int diffuseNr = 1;		// 漫反射纹理序号
    unsigned int specularNr = 1;	// 镜面反射纹理序号
    unsigned int emissionNr = 1;	// 自发光纹理序号
    for(unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i); 							// 激活相应纹理单元
        std::string number;
        std::string name = textures[i].type;						// 获取网格数据中的纹理名称
        if(name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if(name == "texture_specular")
            number = std::to_string(specularNr++);
        else if(name == "texture_emission")
            number = std::to_string(emissionNr++);					// 纹理分别计数

        shader.set1Int(("material." + name + number).c_str(), i);	// 传入纹理
        glBindTexture(GL_TEXTURE_2D, textures[i].id);				// 绑定纹理
    }
    glActiveTexture(GL_TEXTURE0);	// 重置纹理为默认一号纹理

    glBindVertexArray(VAO);			// 启用指定 VAO 进行绘制
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::setupMesh() {
	glGenVertexArrays(1, &VAO);			// 生成顶点数组对象
    glGenBuffers(1, &VBO);				// 生成顶点缓冲对象
    glGenBuffers(1, &EBO);				// 生成元素缓冲对象（元素绘制索引）

    glBindVertexArray(VAO);				// 绑定顶点数组对象
    glBindBuffer(GL_ARRAY_BUFFER, VBO);	// 绑定顶点缓冲对象

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);				// 描述顶点数组
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);	// 绑定元素索引
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);	// 描述索引数组
	
	glEnableVertexAttribArray(0);				// 定位传递位置 1
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);	// 传递顶点数组的位置向量
    glEnableVertexAttribArray(1);				// 定位传递位置 2
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));		// 传递顶点数组的法线向
    glEnableVertexAttribArray(2);				// 定位传递位置 3
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));	// 传递顶点数组的纹理坐标

    glBindVertexArray(0);						// 解除 VAO 绑定
}