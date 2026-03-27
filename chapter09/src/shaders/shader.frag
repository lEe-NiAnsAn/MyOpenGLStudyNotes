#version 330 core

struct Material {		// 材质属性
    sampler2D diffuse;	// 漫反射与环境光颜色（不透明类型纹理）
    sampler2D specular;	// 镜面反射光颜色
    sampler2D emission;	// 物体自发光（放射光纹理）
    float shininess;	// 镜面光泽度
}; 
struct Light {			// 光源属性
    vec3 position;		// 光源位置
    vec3 ambient;		// 对环境光的分量（环境光的强度）
    vec3 diffuse;		// 对漫反射光的分量（漫反射光的强度）
    vec3 specular;		// 对镜面反射光的分量（镜面反射光的强度）
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform int colorFlag;
uniform Material material;
uniform Light light;
uniform vec3 lineColor;
uniform vec3 viewPos;

out vec4 FragColor;

void main() {
	// 从传入的纹理中根据纹理坐标取出对应位置的 vec4 颜色值
	vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;	// 纹理环境光照

	vec3 emission = texture(material.emission, TexCoords).rgb;	// 放射光纹理

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(light.position - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);	// 漫反射系数
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;	// 纹理漫反射光照

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);	// 高光系数
	vec3 specular = light.specular * (spec * texture(material.specular, TexCoords).rgb);	// 纹理镜面反射光照

	FragColor = vec4(ambient + diffuse + specular + emission, 1.0);
}