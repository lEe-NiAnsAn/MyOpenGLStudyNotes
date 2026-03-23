#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 time;

out vec3 myColor;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1.0);	// 从右向左读写依次相乘
	myColor = aColor;
}