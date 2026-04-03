/*
如何使用 CMake 通过 minGW-w64 编译 assimp 库：
1. 安装、配置 minGW-w64、Cmake；
2. 下载 assimp 源码；
3. 打开 Cmake GUI ，配置源码路径与构建路径；
4. 使用【MinGW Makefiles】作为生成器，并选择【Specify native compilers】配置 gcc 与 g++ 路径；
5. 勾选【BUILD_SHARED_LIBS】（生成 DLL）；
6. 取消勾选【ASSIMP_BUILD_TESTS】、【ASSIMP_BUILD_ASSIMP_TOOLS】（不构建测试程序）；
7. 修改【CMAKE_INSTALL_PREFIX】为指定构建路径；
8. 修改【CMAKE_BUILD_TYPE】为 Release；
9. 点击 【Configure】、【Generate】生成 Makefile；
10. 打开 cmd 进入构建路径，执行 “mingw32-make -j8” 编译，再执行 “mingw32-make install” 安装。
*/

#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

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
    void Draw (Shader shader);	// 绘制模型

private:
    std::vector<Texture> textures_loaded;	// 已加载纹理储存容器
    std::vector<Mesh> meshes;   // 网格容器
    std::string directory;      // 模型目录

    void loadModel(const std::string path);                 // 加载模型
    void processNode(aiNode *node, const aiScene *scene);   // 递归处理节点
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);   // 转换网格类型至自定义网格类
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,  std::string typeName);  // 加载材质纹理
    unsigned int TextureFromFile(const char *path, std::string &directory);    // 返回指定路径纹理 ID
};

#endif