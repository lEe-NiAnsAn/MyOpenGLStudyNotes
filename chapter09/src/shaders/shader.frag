#version 330 core

struct Material {		// 材质属性
    sampler2D diffuse;	// 漫反射与环境光颜色（不透明类型纹理）
    sampler2D specular;	// 镜面反射光颜色
    sampler2D emission;	// 物体自发光（放射光纹理）
    float shininess;	// 镜面光泽度
}; 
struct Light {			// 光源属性
    vec3 position;		// 光源位置
	vec3 direction;		// 光照方向
	float innerCutOff;	// 聚光切内光角余弦值
	float outerCutOff;	// 聚光切外光角余弦值
	float constant;		// 光照衰减公式常数项
    float linear;		// 光照衰减公式一次项
    float quadratic;	// 光照衰减公式二次项
    vec3 ambient;		// 对环境光的分量（环境光的强度）
    vec3 diffuse;		// 对漫反射光的分量（漫反射光的强度）
    vec3 specular;		// 对镜面反射光的分量（镜面反射光的强度）
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform int colorFlag;
uniform int lightFlag;
uniform int lightMode;
uniform Material material;
uniform Light light;
uniform vec3 lineColor;
uniform vec3 viewPos;

out vec4 FragColor;

void main() {
	// 从传入的纹理中根据纹理坐标取出对应位置的 vec3 颜色值
	vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;	// 纹理环境光照
	vec3 emission = texture(material.emission, TexCoords).rgb;	// 放射光纹理
	vec3 norm = normalize(Normal);	// 顶点法线向量归一化
	vec3 lightDir;
	if (lightMode == 3 || lightMode == 5) {
		lightDir = normalize(light.position - FragPos);
	}
	if (lightMode == 4) {
		lightDir = normalize(-light.direction);
	}

	float diff = max(dot(norm, lightDir), 0.0);	// 漫反射系数
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;	// 纹理漫反射光照

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);	// 高光系数
	vec3 specular = light.specular * (spec * texture(material.specular, TexCoords).rgb);	// 纹理镜面反射光照

	if (lightMode == 5) {	// 聚光源照射边缘软化
		float theta = dot(lightDir, normalize(-light.direction));	// 物体某点与光源连线较该平面与光源垂线的夹角
		float epsilon = light.innerCutOff - light.outerCutOff;	// 内外切光角余弦值之差
		// 插值公式计算，并限定在[0, 1]之间，得到内外余弦值（即内外光切角）之间根据theta角的平滑变化
		float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);	
		diffuse  *= intensity;
		specular *= intensity;
	}
	
	float distance = length(light.position - FragPos);	// 光源距离
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));	// 光照衰弱公式
	diffuse  *= attenuation;
	specular *= attenuation;

	FragColor = vec4(ambient + diffuse + specular + emission, 1.0);
}