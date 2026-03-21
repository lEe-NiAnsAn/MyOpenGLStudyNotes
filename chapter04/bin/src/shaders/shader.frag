#version 330 core

in vec3 myColor;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 time;
uniform sampler2D myTexture01;
uniform sampler2D myTexture02;

void main() {
	vec3 tempTime = time;
	tempTime.x = abs(sin(1.7 * tempTime.x)/1.7f);
	tempTime.y = abs(cos(1.3 * tempTime.y)/2.3f);
	tempTime.z = abs(sin(0.7 * tempTime.z)/1.3f);
	vec3 tempColor = myColor + tempTime;
	tempColor = min(tempColor, vec3(1.0));
	// 采样纹理颜色混合顶点颜色（ mix 函数可接受两个值，并通过第三个参数表示后者所占比例进行线性插值混合）
	FragColor = mix(texture(myTexture01, TexCoord), texture(myTexture02, TexCoord), 0.4) * vec4(tempColor, 0.3 + (abs(sin(0.5 * time.x)) / 2.0f));
}