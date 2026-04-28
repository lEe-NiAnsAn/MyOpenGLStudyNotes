#version 330 core

layout(std140) uniform Light {
    float innerCutOff;	// 聚光切内光角余弦值
    float outerCutOff;	// 聚光切外光角余弦值
    float constant;		// 光照衰减公式常数项
    float linear;		// 光照衰减公式一次项
    float quadratic;	// 光照衰减公式二次项
    vec3 position;		// 光源位置
    vec3 direction;		// 光照方向
    vec3 diffuse;		// 对漫反射光的分量（漫反射光的强度）
    vec3 specular;		// 对镜面反射光的分量（镜面反射光的强度）
    vec3 ambient;		// 对环境光的分量（环境光的强度）
} light;

in VS_OUT {
	vec3 FragPos;
	vec3 Color;
	vec3 Normal;
} fs_in;

uniform vec2 screenSize;

out vec4 FragColor;

void main() {
	vec3 orientedNormal = fs_in.Normal;
	if (!gl_FrontFacing) {			// 使用 gl_FrontFacing 检测是否面向正面，实现背面时反转法线
		orientedNormal = -orientedNormal;   
	}
	vec3 norm = normalize(orientedNormal);	// 归一化

	vec3 ambient = light.ambient * fs_in.Color;	// 纹理环境光照

	vec3 lightDir = normalize(light.position - fs_in.FragPos);	// 光线方向
	float diff = max(dot(norm, lightDir), 0.0);					// 漫反射系数
	vec3 diffuse = light.diffuse * diff * fs_in.Color;			// 纹理漫反射光照

	vec3 reflectDir = reflect(-lightDir, norm);							// 反射方向
	float spec = pow(max(dot(lightDir, reflectDir), 0.0), 12);			// 高光系数
	vec3 specular = light.specular * spec * fs_in.Color;				// 纹理镜面反射光照

	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.innerCutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);	// 插值公式计算
	diffuse  *= intensity;
	specular  *= intensity;
	
	float distance = length(light.position - fs_in.FragPos);	// 光源距离
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));	// 光照衰弱公式
	diffuse  *= attenuation;
	specular  *= attenuation;
	bool xR = gl_FragCoord.x < (screenSize.x / 2 + 18.0);
	bool xL = gl_FragCoord.x > (screenSize.x / 2 - 18.0);
	bool xT = gl_FragCoord.y < (screenSize.y / 2 +  1.8);
	bool xB = gl_FragCoord.y > (screenSize.y / 2 -  1.8);
	bool yT = gl_FragCoord.y < (screenSize.y / 2 + 18.0);
	bool yB = gl_FragCoord.y > (screenSize.y / 2 - 18.0);
	bool yR = gl_FragCoord.x < (screenSize.x / 2 +  1.8);
	bool yL = gl_FragCoord.x > (screenSize.x / 2 -  1.8);
	if (((xR && xL) && (xT && xB)) || ((yR && yL) && (yT && yB))) {
		FragColor = vec4(1.0 - (ambient + diffuse + specular), 1.0);	// 在屏幕中心显示反色十字准星（仅投射在物体上时可见）
	}
	else {
		FragColor = vec4(ambient + diffuse + specular, 1.0);
	}
}