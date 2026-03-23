#version 330 core

in vec3 myColor;

out vec4 FragColor;

uniform vec3 time;

void main() {
	vec3 tempTime = time;
	tempTime.x = abs(sin(1.7 * tempTime.x)/1.7f);
	tempTime.y = abs(cos(1.3 * tempTime.y)/2.3f);
	tempTime.z = abs(sin(0.7 * tempTime.z)/1.3f);
	vec3 tempColor = myColor + tempTime;
	tempColor = min(tempColor, vec3(1.0));
	FragColor = vec4(tempColor, 0.3 + (abs(sin(0.5 * time.x)) / 2.0f));
}