#version 330 core

struct Material {	// 材质属性
    vec3 ambient;		// 环境光颜色
    vec3 diffuse;		// 漫反射光颜色（固有色）
    vec3 specular;		// 镜面反射光颜色
    float shininess;	// 镜面光泽度
}; 
struct Light {	// 光源属性
    vec3 position;		// 光源位置
    vec3 ambient;		// 对环境光的分量（环境光的强度）
    vec3 diffuse;		// 对漫反射光的分量（漫反射光的强度）
    vec3 specular;		// 对镜面反射光的分量（镜面反射光的强度）
};

in vec3 Normal;
in vec3 FragPos;

uniform int colorFlag;
uniform Material material;
uniform Light light;
uniform vec3 lineColor;
uniform vec3 viewPos;

out vec4 FragColor;

void main() {
	vec3 result = vec3(1.0);

	vec3 ambient = light.ambient * material.ambient;	// 环境光照

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(light.position - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);	// 漫反射系数等于法向量与光线方向的点积，即二者所呈角度的 cos 值，并限定不小于 0
	vec3 diffuse = light.diffuse * (diff * material.diffuse);

	vec3 viewDir = normalize(viewPos - FragPos);	// 摄像机所摄单位方向
	vec3 reflectDir = reflect(-lightDir, norm);		// 传入光线反方向与法线向量可得到反射向量
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);	// 物体的光泽度反映镜面反射强度，即高光系数
	vec3 specular = light.specular * (spec * material.specular);

	switch (colorFlag) {
	case 1:				// 表面颜色
		result = ambient + diffuse + specular;
		break;
	case 2:				// 线框颜色
		result = lineColor;
		break;
	default:
		break;
	}
	FragColor = vec4(result, 1.0);
}