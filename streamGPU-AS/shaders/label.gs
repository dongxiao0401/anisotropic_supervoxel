#version 430
layout(points )in;
layout(points ,max_vertices = 1 )out;

uniform sampler3D texels;

in VS_OUT {
	int gLayer;
	vec3 gTex;
} gs_in[];

flat out int f_label_i;

void main(){
	gl_Position=gl_in[0].gl_Position;
	gl_Layer = gs_in[0].gLayer;
	
	vec4 seed = texture(texels,gs_in[0].gTex);
	f_label_i = int(seed.w);
	
	EmitVertex();
	EndPrimitive();
}