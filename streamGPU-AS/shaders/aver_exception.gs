#version 430
layout(points )in;
layout(points ,max_vertices = 1 )out;

in VS_OUT {
	int gLayer;
	vec4 gCentroidPos;
	vec3 gCentroidColor;
} gs_in[];

out vec4 f_pos;
out vec3 f_color;

void main(){
	gl_Position=gl_in[0].gl_Position;
	gl_Layer = gs_in[0].gLayer;
	f_pos = gs_in[0].gCentroidPos;
	f_color = gs_in[0].gCentroidColor;
	EmitVertex();
	EndPrimitive();
}