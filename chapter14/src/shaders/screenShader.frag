#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform int postPrecessMode;
uniform vec2 offset;

void main() {
    vec3 color = texture(screenTexture, TexCoords).rgb;
	// 偏移量数组，即卷积核内像素点坐标
    vec2 offsets[9] = vec2[](	// 九宫格卷积核
        vec2(-offset.x,	 offset.y), 	// 左上
        vec2( 0.0f,    	 offset.y),		// 正上
        vec2( offset.x,	 offset.y), 	// 右上
        vec2(-offset.x,	 0.0f),     	// 左
        vec2( 0.0f,		 0.0f),       	// 中
        vec2( offset.x,	 0.0f),			// 右
        vec2(-offset.x, -offset.y), 	// 左下
        vec2( 0.0f,		-offset.y), 	// 正下
        vec2( offset.x, -offset.y)  	// 右下
    );
	// 纹理采样值数组，即当前卷积核所在区域
	vec3 sampleTex[9];	
    for(int i = 0; i < 9; i++) {
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));	// 读取以该纹理坐标为中心的九宫格区域像素值
    }
	// 卷积核，即核内像素权重
	float sharpen[9] = float[](	// 锐化卷积核（基础拉普拉斯锐化）
         0.0, -1.0,  0.0,
        -1.0,  5.0, -1.0,
         0.0, -1.0,  0.0
    );
	float gaussian[9] = float[](	// 高斯模糊卷积核（3×3轻度模糊核）
        1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
        2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0,
        1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0
    );
	float edgeDetection[9] = float[](	// 边缘检测卷积核（拉普拉斯对角线算子）
        1.0,  1.0, 1.0,
        1.0, -8.0, 1.0,
        1.0,  1.0, 1.0
    );
	float emboss[9] = float[](	// 浮雕效果卷积核
        -2.0, -1.0, 0.0,
    	-1.0,  1.0, 1.0,
         0.0,  1.0, 2.0
    );

    switch (postPrecessMode) {
        case 0: // 无处理
            break;
        case 1: // 反色
            color = 1.0 - color;
            break;
        case 2: // 灰度
            float average = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;	// 加权通道灰度
			color = vec3(average);
            break;
        case 3: // 锐化
			color = vec3(0.0f);	// 临时颜色值置空
            for(int i = 0; i < 9; i++)
        		color += sampleTex[i] * sharpen[i];	// 卷积运算
            break;
        case 4: // 模糊
			color = vec3(0.0f);
            for(int i = 0; i < 9; i++)
        		color += sampleTex[i] * gaussian[i];
            break;
        case 5: // 边缘检测
			color = vec3(0.0f);
            for(int i = 0; i < 9; i++)
        		color += sampleTex[i] * edgeDetection[i];
            break;
        case 6: // 浮雕效果
			color = vec3(0.0f);
            for(int i = 0; i < 9; i++)
        		color += sampleTex[i] * emboss[i];
            break;
        default:
            break;
    }
    FragColor = vec4(color, 1.0);
} 