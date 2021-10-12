#version 430
layout(points )in;
layout(points ,max_vertices = 1 )out;

in VS_OUT {
	vec4 gSeedPosId;
	vec3 gSeedColor;
	vec3 gSeedMF3;
	vec3 gSeedML3;
} gs_in[];  


out vec4 f_pos;
out vec3 f_color;
out vec3 f_MF3;
out vec3 f_ML3;


void main(){
	
	gl_Position=gl_in[0].gl_Position;//这里是gl_in[0]
	gl_Layer = int(gs_in[0].gSeedPosId.z);
	
	f_pos = gs_in[0].gSeedPosId;
	f_color = gs_in[0].gSeedColor;
	
	f_MF3 = gs_in[0].gSeedMF3;
	f_ML3 = gs_in[0].gSeedML3;
	
	EmitVertex();
	EndPrimitive();
}
