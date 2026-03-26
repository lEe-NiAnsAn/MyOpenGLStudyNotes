#version 330 core

in vec3 Normal;
in vec3 FragPos;

uniform int colorFlag;
uniform vec3 objectColor;
uniform vec3 lineColor;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform float specularStrength;
uniform vec3 lightPos;
uniform vec3 viewPos;

out vec4 FragColor;

void main() {
	vec3 result = vec3(1.0);

	vec3 ambient = ambientStrength * lightColor;	// 环境光照

	vec3 norm = Normal;
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(0.8f * dot(norm, lightDir), 0.0);	// 漫反射系数等于法向量与光线方向的点积，即二者所呈角度的 cos 值，并限定不小于 0
	vec3 diffuse = diff * lightColor;

	vec3 viewDir = normalize(viewPos - FragPos);	// 摄像机所摄单位方向
	vec3 reflectDir = reflect(-lightDir, norm);		// 传入光线反方向与法线向量可得到反射向量
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);	// 镜面反射系数的32次幂，即高光的反光度
	vec3 specular = specularStrength * spec * lightColor;

	switch (colorFlag) {
	case 1:				// 表面颜色
		result = objectColor * (specular + ambient + diffuse);
		break;
	case 2:				// 线框颜色
		result = lineColor * (ambient + diffuse);
		break;
	default:
		break;
	}
	FragColor = vec4(result, 1.0);
}