#version 330 core

layout (location = 0) in vec3 aPos;	// 位置变量属性位置值为0
layout (location = 1) in vec3 aColor;	// 颜色变量属性位置值为1

out vec3 myColor;	// 向片段着色器输出一个颜色

void main() {
	gl_Position = vec4(aPos, 1.0);
	myColor = aColor;	// 设置输出颜色为顶点数据中的值
}