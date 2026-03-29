#version 330 core

struct Material {			// 材质属性
    sampler2D diffuse;		// 漫反射与环境光颜色（不透明类型纹理）
    sampler2D specular;		// 镜面反射光颜色
    sampler2D emission;		// 物体自发光（放射光纹理）
    float shininess;		// 镜面光泽度
};
struct PointLight {			// 点光属性
    vec3 position;			// 光源位置

    float constant;			// 常数项
    float linear;			// 一次项
    float quadratic;		// 二次项				
    vec3 ambient;			// 环境光分量
    vec3 diffuse;			// 漫反射光分量
    vec3 specular;			// 镜面反射光分量
};
struct ParalleltLight {		// 平行光属性
    vec3 direction;			// 光照方向

    float constant;			// 常数项
    vec3 ambient;			// 环境光分量
    vec3 diffuse;			// 漫反射光分量
    vec3 specular;			// 镜面反射光分量
};
struct SpotLight {			// 聚光属性
    vec3 position;			// 光源位置
    vec3 direction;			// 光照方向
    float innerCutOff;		// 内切光角
    float outerCutOff;		// 外切光角

    float constant;			// 常数项
    float linear;			// 一次项
    float quadratic;		// 二次项				
    vec3 ambient;			// 环境光分量
    vec3 diffuse;			// 漫反射光分量
    vec3 specular;			// 镜面反射光分量
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform int colorFlag;
uniform int lightFlag;
uniform int lightMode;
uniform int flashMode;
uniform Material material;
#define NR_POINT_LIGHTS 2	// 预处理指令定义了场景中点光源数量
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform ParalleltLight paralleltLight;
uniform SpotLight spotLight;
uniform vec3 lineColor;
uniform vec3 viewPos;

out vec4 FragColor;

vec3 CalcPointLight(PointLight pointLight, vec3 norm, vec3 fragPos, vec3 viewDir);
vec3 CalcParalleltLight(ParalleltLight paralleltLight, vec3 normal, vec3 viewDir);
vec3 CalcSpotLight(SpotLight spotLight, vec3 norm, vec3 fragPos, vec3 viewDir);


void main() {
	vec3 viewDir = normalize(viewPos - FragPos);	// 摄像机所摄方向
	vec3 norm = normalize(Normal);	// 顶点法线向量归一化
	vec3 result;

	if (lightMode == 3) {
		result += CalcPointLight(pointLights[0], norm, FragPos, viewDir);
		result += CalcPointLight(pointLights[1], norm, FragPos, viewDir);
	}
	else if (lightMode == 4) {
		result += CalcParalleltLight(paralleltLight, norm, viewDir);
	}
	if (!(flashMode == 6)) {
		result += CalcSpotLight(spotLight, norm, FragPos, viewDir);	// 叠加聚光照射
	}

	vec3 emission = texture(material.emission, TexCoords).rgb;	// 放射光纹理
	FragColor = vec4(result + emission, 1.0);
}

vec3 CalcPointLight(PointLight pointLight, vec3 norm, vec3 fragPos, vec3 viewDir) {
	vec3 ambient = pointLight.ambient * texture(material.diffuse, TexCoords).rgb;	// 纹理环境光照

	vec3 lightDir = normalize(pointLight.position - fragPos);	// 光线方向

	float diff = max(dot(norm, lightDir), 0.0);	// 漫反射系数
	vec3 diffuse = pointLight.diffuse * diff * texture(material.diffuse, TexCoords).rgb;	// 纹理漫反射光照

	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);	// 高光系数
	vec3 specular = pointLight.specular * (spec * texture(material.specular, TexCoords).rgb);	// 纹理镜面反射光照

	float distance = length(pointLight.position - fragPos);	// 光源距离
	float attenuation = 1.0 / (pointLight.constant + pointLight.linear * distance + pointLight.quadratic * (distance * distance));	// 光照衰弱公式
	diffuse  *= attenuation;
	specular *= attenuation;

	return vec3(ambient + diffuse + specular);
}

vec3 CalcParalleltLight(ParalleltLight paralleltLight, vec3 norm, vec3 viewDir) {
	vec3 ambient = paralleltLight.ambient * texture(material.diffuse, TexCoords).rgb;	// 纹理环境光照

	vec3 lightDir = normalize(-paralleltLight.direction);

	float diff = max(dot(norm, lightDir), 0.0);	// 漫反射系数
	vec3 diffuse = paralleltLight.diffuse * diff * texture(material.diffuse, TexCoords).rgb;	// 纹理漫反射光照

	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);	// 高光系数
	vec3 specular = paralleltLight.specular * (spec * texture(material.specular, TexCoords).rgb);	// 纹理镜面反射光照

	return vec3(ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight spotLight, vec3 norm, vec3 fragPos, vec3 viewDir) {
	vec3 lightDir = normalize(spotLight.position - FragPos);

	float diff = max(dot(norm, lightDir), 0.0);	// 漫反射系数
	vec3 diffuse = spotLight.diffuse * diff * texture(material.diffuse, TexCoords).rgb;	// 纹理漫反射光照

	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);	// 高光系数
	vec3 specular = spotLight.specular * (spec * texture(material.specular, TexCoords).rgb);	// 纹理镜面反射光照

	float theta = dot(lightDir, normalize(-spotLight.direction));
	float epsilon = spotLight.innerCutOff - spotLight.outerCutOff;
	float intensity = clamp((theta - spotLight.outerCutOff) / epsilon, 0.0, 1.0);	// 平滑插值
	diffuse  *= intensity;
	specular *= intensity;

	float distance = length(spotLight.position - fragPos);	// 光源距离
	float attenuation = 1.0 / (spotLight.constant + spotLight.linear * distance + spotLight.quadratic * (distance * distance));	// 光照衰弱公式
	diffuse  *= attenuation;
	specular *= attenuation;

	return vec3(diffuse + specular);
}
