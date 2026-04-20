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

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_emission1;
};
uniform Material material;
uniform Light light;
uniform samplerCube skybox;
uniform int reflectMode;

out vec4 FragColor;

void main() {
	if (reflectMode == 0) {
		vec3 orientedNormal = Normal;	// 双面法线
		if (!gl_FrontFacing) {			// 背面时反转法线
			orientedNormal = -orientedNormal;   
		}
		vec3 norm = normalize(orientedNormal);	// 归一化

		vec3 ambient = light.ambient * 
						(texture(material.texture_diffuse1, TexCoords).rgb + 
						texture(material.texture_emission1, TexCoords).rgb);						// 纹理环境光照

		vec3 lightDir = normalize(light.position - FragPos);										// 光照方向（由物体至摄像机）
		float diff = max(dot(norm, lightDir), 0.35);												// 漫反射系数
		vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, TexCoords).rgb;	// 纹理漫反射光照

		vec3 reflectDir = reflect(-lightDir, norm);													// 反射方向
		float spec = pow(max(dot(lightDir, reflectDir), 0.0), 16);									// 高光系数
		vec3 specular = light.specular * spec * texture(material.texture_diffuse1, TexCoords).rgb;	// 纹理镜面反射光照

		vec3 emission = texture(material.texture_emission1, TexCoords).rgb;							// 自发光纹理

		float theta = dot(lightDir, normalize(-light.direction));
		float epsilon = light.innerCutOff - light.outerCutOff;
		float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);	// 插值公式计算
		diffuse  *= intensity;
		specular  *= intensity;
		
		float distance = length(light.position - FragPos);	// 光源距离
		float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));	// 光照衰弱公式
		diffuse  *= attenuation;
		specular  *= attenuation;
		emission  *= attenuation;
		
		FragColor = vec4(ambient + diffuse + emission + specular, 1.0);
	}

	if (reflectMode == 1) {
		vec3 I = normalize(FragPos - light.position);			// 反射光射入方向
		vec3 R = reflect(I, normalize(Normal));					// 反射光初射方向
		FragColor = vec4(0.9 * texture(skybox, R).rgb, 1.0);	// 使用天空盒纹理表示反射效果，该重载函数可通过方向向量 R 计算被其“击中的面”的 UV 坐标并转换到相应的 UVW 坐标
	}

	if (reflectMode == 2) {
		float ratio = 1.00 / 1.52;	// 折射率比值
		vec3 I = normalize(FragPos - light.position);
		vec3 Re = reflect(I, normalize(Normal));		// 反射方向
		vec3 reflectColor = texture(skybox, Re).rgb;	// 反射
		vec3 Ra = refract(I, normalize(Normal), ratio);	// 折射方向	
		vec3 refractColor = texture(skybox, Ra).rgb;	// 水面折射（单次折射）
		FragColor = vec4(0.8 * refractColor + 0.02 * reflectColor, 1.0);
	}
}