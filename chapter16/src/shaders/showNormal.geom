#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

layout (std140) uniform Matrices {
	mat4 projection;			
	mat4 view;
};
uniform mat4 model;

in VS_OUT {
    vec3 FragPos;
	vec3 Color;
	vec3 Normal;    // 从该处获取顶点数组包含的法线向量数据
} gs_in[];

out VS_OUT {    // 衔接后续片段着色器数据
    vec3 FragPos;
	vec3 Color;
	vec3 Normal;
} gs_out;

const float MAGNITUDE = 0.4;    // 所显示的法线的长度

void GenerateLine(int index)
{
    gs_out.FragPos = gs_in[index].FragPos;
    gs_out.Color   = vec3(0.0, 1.0, 0.0);   // 可视化法线颜色
    gs_out.Normal  = gs_in[index].Normal;   // 传输原法线供片段着色器使用
    gl_Position = gl_in[index].gl_Position; // 可视化法线端点一（顶点）
    EmitVertex();
    mat3 normalMatrix = mat3(transpose(inverse(view * model))); // 法线转换矩阵：观察矩阵转置后取反并截取 3*3 范围
    vec3 normal = normalize(vec3(vec4(normalMatrix * gs_in[index].Normal, 0.0)));   // 将法线坐标系转换至观察空间，避免法线永远指向摄像头
    gl_Position = gl_in[index].gl_Position + projection * vec4(normal, 0.0) * MAGNITUDE;    // 可视化法线端点二
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(0); // 顶点一初始法线
    GenerateLine(1); // 顶点二初始法线
    GenerateLine(2); // 顶点三初始法线
}