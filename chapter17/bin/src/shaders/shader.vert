#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;

uniform mat4 model;
layout (std140) uniform Matrices {
	mat4 projection;
	mat4 view;
};
layout (std140) uniform InstanceOffsets {
    vec3 offsets[125];	// 实例的位置偏移量数组，预设125
} Offsets;

out VS_OUT {
	vec3 FragPos;
	vec3 Color;
	vec3 Normal;
} vs_out;

void main() {
	vec3 offset = Offsets.offsets[gl_InstanceID];
	gl_Position = projection * view * model * vec4(aPos + offset, 1.0);
	vs_out.Color.r = fract(aColor.r + gl_InstanceID / 12.5);
	vs_out.Color.g = fract(aColor.g + gl_InstanceID / 22.5);
	vs_out.Color.b = fract(aColor.b + gl_InstanceID / 5.5);		// 多种颜色
	vs_out.FragPos = vec3(model * vec4(aPos + offset, 1.0));
	vs_out.Normal = aNormal;
}