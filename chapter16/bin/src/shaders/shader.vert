#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Color;
out vec3 Normal;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	gl_PointSize = clamp(gl_Position.z, 10.0, 100.0);	// 图形顶点随视角距离变远而变大突出，在片段着色器中默认方形扩大，需在主函数中开启相应选项
	Color.r = clamp(aColor.r * (gl_VertexID + 1), 0.0, 1.0);	
	Color.g = clamp(aColor.g * (gl_VertexID + 1), 0.0, 1.0);
	Color.b = clamp(aColor.b * (gl_VertexID + 1), 0.0, 1.0);	// 使用顶点着色器输出变量（当前绘制的顶点索引）修改传入片段着色器的颜色，实现越后绘制的顶点越
	FragPos = vec3(model * vec4(aPos, 1.0));
	Normal = aNormal;
}