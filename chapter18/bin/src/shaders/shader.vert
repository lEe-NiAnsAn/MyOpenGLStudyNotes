#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;

uniform mat4 model;
layout (std140) uniform Matrices {
	mat4 projection;
	mat4 view;
};
out VS_OUT {
	vec3 FragPos;	// 世界空间坐标
	vec3 Color;
	vec3 Normal;
} vs_out;

void main() {
	vec4 worldPos = model* vec4(aPos, 1.0);	// 世界空间坐标
	gl_Position = projection * view * worldPos;	// 裁剪空间坐标
	vs_out.Color = aColor;
	vs_out.FragPos = worldPos.xyz;
	vs_out.Normal = aNormal;
}