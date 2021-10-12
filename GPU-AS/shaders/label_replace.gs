#version 430
layout(points )in;
layout(points ,max_vertices = 1 )out;

uniform sampler3D texels;

uniform int dead_seed_index;
uniform int last_seed_index;


in VS_OUT {
	vec3 gPos;
	vec3 gTex;
} gs_in[];

out vec4 f_color;

void main(){
	gl_Position=gl_in[0].gl_Position;//这里是gl_in[0]
	gl_Layer = int(gs_in[0].gPos.z);//z为0.5，gl_Layer=0
	vec3 curr_xyz = gs_in[0].gPos;
	vec3 curr_tex = gs_in[0].gTex;//texture坐标
	
	vec4 curr_seed = texture(texels,curr_tex);
	f_color = curr_seed;
	if(int(curr_seed.w)==last_seed_index)
	{
		f_color.w = dead_seed_index;
	}
	
	
	EmitVertex();
	EndPrimitive();
}