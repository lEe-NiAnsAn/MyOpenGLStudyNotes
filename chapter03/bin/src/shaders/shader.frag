#version 330 core

out vec4 FragColor;

in vec3 myColor;	// 接收初始颜色值

uniform vec3 time;  // 【全局变量】控制颜色变化

void main() {
	vec3 tempTime = time;
	// 三角函数“潮汐”变化修改值
	tempTime.x = abs(sin(1.7 * tempTime.x)/1.7f);
	tempTime.y = abs(cos(1.3 * tempTime.y)/2.3f);
	tempTime.z = abs(sin(0.7 * tempTime.z)/1.3f);
	vec3 tempColor = myColor + tempTime;	// 初始值与变量值相加取浮点数绝对值
	tempColor = min(tempColor, vec3(1.0));	// 防止溢出
	FragColor = vec4(tempColor, 1.0);	// 无法直接修改由顶点着色器输入的变量属性
}