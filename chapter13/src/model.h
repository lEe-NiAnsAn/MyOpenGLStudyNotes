#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "stb/stb_image.h"

#include "mesh.h"
#include "shader.h"

class Model {
public :
    Model(const char *path);	// 构造函数
    void Draw (Shader shader, const glm::vec3& viewPos);	// 绘制模型

private:
    std::vector<Texture> textures_loaded;  	// 已加载纹理储存容器
    std::vector<Mesh> opaqueMeshes;        	// 无透明网格容器
    std::vector<Mesh> transparentMeshes;   	// 透明度网格容器
    std::string directory;                  // 模型目录

    void loadModel(const std::string path);                 // 加载模型
    void processNode(aiNode *node, const aiScene *scene);   // 递归处理节点
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);   // 转换网格类型至自定义网格类
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,  std::string typeName);  // 加载材质纹理
    unsigned int TextureFromFile(const char *path, std::string &directory);    // 返回指定路径纹理 ID
};

#endif