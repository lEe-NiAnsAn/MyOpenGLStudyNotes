#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;  //  将纹理 uniform 采样器设置为 sampler2DMS 可获取每个子样本的颜色值
uniform int multiNum;

void main() {
    vec3 color = texture(screenTexture, TexCoords).rgb;
    // vec4 colorSample4 = texelFetch(screenTextureMS, TexCoords, 3);  // sampler2DMS 采样的第4个子样本（子样本位置具体排列位置不定，通常抖动分布）
    // 此时便可通过子样本级别处理实现边缘检测 MSAA 等优化的抗锯齿算法，提高性能
    color.r += (1.0 - color.r) / 32.0 * multiNum;       // 采样倍率越高屏幕越发白
    color.g += (1.0 - color.g) / 32.0 * multiNum;
    color.b += (1.0 - color.b) / 32.0 * multiNum;
    FragColor = vec4(color, 1.0);
} 