#version 330 core

layout (points) in; // 传入绘制图元类型（单个顶点）（也可传入 triangles 等）
layout (points, max_vertices = 2) out;  // 传出绘制图元方式并设定最大数目（绘制两个顶点）（也可绘制线段、三角）

layout (std140) uniform Matrices {
	mat4 projection;			
	mat4 view;
};

// 几何着色器接收对象为图元（顶点数组）
in VS_OUT {
    vec3 FragPos;
	vec3 Color;
	vec3 Normal;
} gs_in[];

out VS_OUT {
    vec3 FragPos;
	vec3 Color;
	vec3 Normal;
} gs_out;

void main() {
    gs_out.FragPos = gs_in[0].FragPos;
    gs_out.Color   = gs_in[0].Color;
    gs_out.Normal  = gs_in[0].Normal;
    gl_Position    = gl_in[0].gl_Position;  // 可在此对顶点进行操作，且可重复使用（例如重复使用三次、每次更改顶点坐标，以进行三角形的绘制）
	gl_PointSize   = clamp(gl_Position.z, 10.0, 100.0);	// 当不绘制三角形而仅绘制顶点时，图形顶点随视角距离变远而变大突出，在片段着色器中默认方形扩大，需在主函数中开启相应选项
    EmitVertex();// 每次调用均会将 gl_Position 的顶点数据添加至待绘制图元中
    // 当绘制方式为线段或三角时，可遍历加载 gs_in 各个顶点从而绘制
    EndPrimitive(); // 合成所有发射出的顶点数据为指定的输出渲染图元

    gs_out.Color   = vec3(0.0, 1.0, 0.0);
    gl_Position    = projection * view * (vec4(gs_in[0].FragPos, 1.0) + vec4(0.0, 0.0, 1.0, 0.0));
	gl_PointSize   = clamp(gl_Position.z, 10.0, 100.0);
    EmitVertex();
    EndPrimitive(); // 重复调用生成多个图元
}