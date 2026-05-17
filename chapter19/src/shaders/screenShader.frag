#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform int multiNum;
uniform bool gammaFlag;
uniform float gammaValue;

void main() {
    vec3 color = texture(screenTexture, TexCoords).rgb;
    FragColor = vec4(color, 1.0);
    // 应用 sRGB 伽马矫正
	if (gammaFlag) {
		for (int i = 0; i < 3; i++) {
			if (FragColor[i] <= 0.0031308) {
				FragColor[i] = 12.92 * FragColor[i];
			}
			else {
				FragColor[i] = 1.055 * pow(FragColor[i], 1.0 / gammaValue) - 0.055;
			}
		}
	}

} 