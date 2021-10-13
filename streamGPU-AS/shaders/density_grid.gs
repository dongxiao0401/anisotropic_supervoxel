#version 430
layout(points )in;
layout(points ,max_vertices = 1 )out;

in VS_OUT {
	int gLayer;
	float p_grad;
} gs_in[];


out float f_grad;

void main(){
	gl_Position=gl_in[0].gl_Position;
	gl_Layer = gs_in[0].gLayer;
	f_grad = gs_in[0].p_grad;
	EmitVertex();
	EndPrimitive();
}