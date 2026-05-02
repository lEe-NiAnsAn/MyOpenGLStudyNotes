#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aOffset;	// 使用顶点属性传入实例偏移量
layout (location = 4) in mat4 instanceMatrix;	// 使用顶点属性传入实例变换矩阵（明确传入 mat4 则只需使用一个属性号，另三个辅助属性号为隐式调用）

layout (std140) uniform Matrices {
	mat4 projection;
	mat4 view;
};

uniform mat4 time_model;

out VS_OUT {
	vec3 FragPos;
	vec3 Color;
	vec3 Normal;
} vs_out;

float speed = 0.1;
void main() {
	vec4 worldPos = time_model * instanceMatrix * vec4(aPos + aOffset, 1.0);	// 世界空间坐标
	gl_Position = projection * view * worldPos;	// 裁剪空间坐标
	vs_out.Color = aColor;
	vs_out.FragPos = worldPos.xyz;
	vs_out.Normal = aNormal;
}