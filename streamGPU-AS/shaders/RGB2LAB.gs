#version 430
layout(points )in;
layout(points ,max_vertices = 1 )out;

uniform sampler3D texels;


in VS_OUT {
	int  gLayer;
	vec3 gTex;
} gs_in[];

out vec3 f_color;



void main(){
	
	gl_Position=gl_in[0].gl_Position;
	gl_Layer = gs_in[0].gLayer;//z为0.5，gl_Layer=0
	vec3 curr_tex = gs_in[0].gTex;
	f_color = texture(texels,curr_tex).rgb;
	EmitVertex();
	EndPrimitive();
}