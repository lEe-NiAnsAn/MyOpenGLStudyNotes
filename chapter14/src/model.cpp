#include "model.h"

Model::Model(const char *path) {
    loadModel(path);
}

void Model::Draw(Shader shader) {
    for (unsigned int i = 0; i < meshes.size(); i++) {	// 遍历绘制所有网格
        meshes[i].Draw(shader);
    }
}

void Model::loadModel(const std::string path) {
    Assimp::Importer importer;  // 声明导入器
    // 读取文件
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);  // 转换为三角形图元成分、翻转y轴纹理坐标
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {	
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;	
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));	// 设置模型路径（截取从开头至最后一个 / 的子字符串）

    processNode(scene->mRootNode, scene);	// 递归处理场景中的所有节点
}

void Model::processNode(aiNode *node, const aiScene *scene) {
	// 处理节点所有网格
    for(unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        meshes.push_back(processMesh(mesh, scene));         
    }
    // 处理其子节点
    for(unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);	// 递归调用自身
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices;		// 顶点数组容器
    std::vector<unsigned int> indices;	// 顶点数组索引
    std::vector<Texture> textures;		// 纹理容器

    for(unsigned int i = 0; i < mesh->mNumVertices; i++) {	// 遍历获取网格所有顶点数组数据
        Vertex vertex;
		// 获取顶点位置向量
		glm::vec3 vector; 
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z; 
		vertex.Position = vector;
		// 获取顶点法线向量
        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
		// 获取顶点纹理坐标
		if (mesh->mTextureCoords[0]) {	// 判断是否存在纹理坐标	
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x; 
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		}
        vertices.push_back(vertex);
    }
    // 获取网格顶点数组的索引数据
    for(unsigned int i = 0; i < mesh->mNumFaces; i++) { // 遍历网格图元面数组
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++) {    // 遍历面数组索引
            indices.push_back(face.mIndices[j]);
        }
    }
    // 获取网格纹理材质数据
    if(mesh->mMaterialIndex >= 0) {	// 判断网格纹理数量是否大于零
		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());	// 末尾插入 diffuseMaps
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());	// 末尾插入 specularMaps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());      // 末尾插入 normalMaps
        std::vector<Texture> emissionMaps = loadMaterialTextures(material, aiTextureType_EMISSIVE, "texture_emission");
        textures.insert(textures.end(), emissionMaps.begin(), emissionMaps.end());      // 末尾插入 emissionMaps
    }

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type,  std::string typeName) {
	std::vector<Texture> textures;	// 纹理容器
    unsigned int count = mat->GetTextureCount(type);
    std::cout << "Loading " << typeName << " textures, count: " << count << std::endl;  // 检查纹理载入情况
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++) {	// 遍历指定类型纹理
        aiString str;
        mat->GetTexture(type, i, &str);	// 获取与指定纹理类型和索引相关的纹理路径
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++) { // 遍历已加载纹理容器
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {    // 比较是否为同一纹理
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
            }
        }
        if (!skip) {
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory);	// 获取纹理图像 ID
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }
    return textures;
}

unsigned int Model::TextureFromFile(const char *path, std::string &directory) {
    std::string filename = directory + '/' + std::string(path);		// 拼接完整路径
	unsigned int textureID;											// 指定纹理 ID
	glGenTextures(1, &textureID);									// 生成纹理
	glBindTexture(GL_TEXTURE_2D, textureID);						// 绑定纹理
    // 载入图像
    int width, height, nrChannels;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
    if(data) {
		GLenum format;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);	// 附着纹理
        glGenerateMipmap(GL_TEXTURE_2D);	                                // 生成多级渐远纹理
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);		// S 轴方向默认重复纹理
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);		// T 轴方向默认重复纹理
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// 线性过滤缩小
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// 线性过滤放大
        stbi_image_free(data);						                        // 释放图像内存
    }
    else {
        std::cout << "Texture failed to load at path: " << filename << std::endl;
    }
	return textureID;
}
