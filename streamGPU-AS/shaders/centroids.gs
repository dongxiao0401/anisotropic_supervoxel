#version 430
layout(points )in;
layout(points ,max_vertices = 1 )out;

in VS_OUT {
	int gLayer;
	vec3 gPos;
	vec3 gColor;
} gs_in[];

out vec3 f_pos;
out vec3 f_color;

void main(){
	gl_Position=gl_in[0].gl_Position;
	gl_Layer = gs_in[0].gLayer;
	f_pos = gs_in[0].gPos;
	f_color = gs_in[0].gColor;
	EmitVertex();
	EndPrimitive();
}