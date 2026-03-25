#version 330 core

in vec3 Normal;
in vec3 FragPos;

uniform int colorFlag;
uniform vec3 objectColor;
uniform vec3 lineColor;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform vec3 lightPos;

out vec4 FragColor;

void main() {
	vec3 ambient = ambientStrength * lightColor;	// 环境光照
	vec3 result = vec3(1.0);
	vec3 norm = Normal;
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);	// 漫反射系数等于法向量与光线方向的点积，即二者所呈角度的 cos 值，并限定不小于 0
	vec3 diffuse = diff * lightColor;
	switch (colorFlag) {
	case 1:				// 表面颜色
		result = objectColor * (ambient + diffuse);
		break;
	case 2:				// 线框颜色
		result = lineColor;
		break;
	default:
		break;
	}
	FragColor = vec4(result, 1.0);
}