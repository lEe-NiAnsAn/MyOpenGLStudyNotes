#version 330 core

struct Light {			// 光源属性
    vec3 position;		// 光源位置
	vec3 direction;		// 光照方向
	float innerCutOff;	// 聚光切内光角余弦值
	float outerCutOff;	// 聚光切外光角余弦值
    vec3 diffuse;		// 对漫反射光的分量（漫反射光的强度）
    vec3 specular;		// 对镜面反射光的分量（镜面反射光的强度）
    vec3 ambient;		// 对环境光的分量（环境光的强度）
	float constant;		// 光照衰减公式常数项
    float linear;		// 光照衰减公式一次项
    float quadratic;	// 光照衰减公式二次项
};

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse;	// 使用四通道而非先前的三通道
uniform Light light;

out vec4 FragColor;

void main() {
	vec3 orientedNormal = Normal;	// 双面法线
	if (!gl_FrontFacing) {			// 背面时反转法线
        orientedNormal = -orientedNormal;   
    }
	vec3 norm = normalize(orientedNormal);	// 归一化

	vec4 myTexture = texture(texture_diffuse, TexCoords);
	vec3 worldUp = vec3(0.0, 1.0, 0.0);							// 世界空间y轴方向
    float upDot = max(dot(norm, worldUp), 0.0);					// 环境光系数
	vec3 amb = mix(vec3(0.8), vec3(1.2), upDot);				// 模拟自然环境光（混合地面系数0.8与天空系数1.2）
	vec3 ambient = light.ambient * amb * myTexture.rgb;			// 纹理环境光照

	vec3 lightDir = normalize(light.position - FragPos);		// 光照方向
	float diff = max(dot(norm, lightDir), 0.0);					// 漫反射系数
	vec3 diffuse = light.diffuse * diff * myTexture.rgb;		// 纹理漫反射光照

	vec3 reflectDir = reflect(-lightDir, norm);					// 反射方向
	float spec = pow(max(dot(lightDir, reflectDir), 0.0), 8.0);	// 高光系数
	vec3 specular = light.specular * spec * myTexture.rgb;		// 纹理镜面反射光照

	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.innerCutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);	// 插值公式计算
	diffuse  *= intensity;
	specular  *= intensity;
	
	float distance = length(light.position - FragPos);	// 光源距离
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));	// 光照衰弱公式
	diffuse  *= attenuation;
	specular  *= attenuation;

	FragColor = vec4((ambient + diffuse + specular), myTexture.a);
}