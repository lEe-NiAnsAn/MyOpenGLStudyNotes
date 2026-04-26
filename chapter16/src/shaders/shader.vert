#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;

uniform mat4 model;
/*
 *	layout(std140, binding = 0) uniform Matrices {  };	// 使用 #version 420 core 可无需在主函数中设置绑定点
 */
// 使用 uniform 缓冲对象传入全局通用变量，实现主函数一次传入，多处着色器程序使用
// UBO 默认使用的共享内存布局需要调用函数方可查询偏移量以进行操作，而非共享布局无法做到由硬件分配的内存布局在多个着色器程序中保持一致
layout (std140) uniform Matrices {	// 使用 std140 显显式声明的共享布局 uniform 块：一般变量的基准对齐量为4字节，记为N；向量则需对齐为2N或4N；数组为数个同vec4大小的元素之和；矩阵则为数个数组大小之和；结构体为计算各部分后填充至vec4的倍数
	mat4 projection;	// 基准对齐量：列0 -> 4N = 16 字节；基准偏移量：0字节，为4N的倍数（例如若此前还有一个 float 变量的四字节对齐量，这里也填充至十六字节偏移量）
						// 			  列1 -> 4N = 16 字节；
						// 			  列2 -> 4N = 16 字节；
						// 			  列3 -> 4N = 16 字节；基准偏移量：64字节
	mat4 view;
};
// 使用接口块传出着色器数据
out VS_OUT {	// 块名：不同着色器中须保持一致
	vec3 FragPos;
	vec3 Color;
	vec3 Normal;
} vs_out;	// 实例名：不同着色器中可自行定义

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	gl_PointSize = clamp(gl_Position.z, 10.0, 100.0);	// 图形顶点随视角距离变远而变大突出，在片段着色器中默认方形扩大，需在主函数中开启相应选项
	vs_out.Color.r = clamp(aColor.r * (gl_VertexID + 1), 0.0, 1.0);	
	vs_out.Color.g = clamp(aColor.g * (gl_VertexID + 1), 0.0, 1.0);
	vs_out.Color.b = clamp(aColor.b * (gl_VertexID + 1), 0.0, 1.0);	// 使用顶点着色器输出变量（当前绘制的顶点索引）修改传入片段着色器的颜色，实现越后绘制的顶点越
	vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
	vs_out.Normal = aNormal;
}