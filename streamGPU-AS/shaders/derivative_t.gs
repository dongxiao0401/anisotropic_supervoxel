#version 430
layout(points )in;
layout(points ,max_vertices = 1 )out;

uniform sampler3D texels;
uniform float width_ratio;
uniform float height_ratio;
uniform float depth_ratio;

in VS_OUT {
	int  gLayer;
	vec3 gTex;//texture坐标，texture()函数使用
} gs_in[];

//out vec3 f_color;
out float gradient_t;


void main(){
	
	gl_Position=gl_in[0].gl_Position;//这里是gl_in[0]
	gl_Layer = gs_in[0].gLayer;//z为0.5，gl_Layer=0
	vec3 curr_tex = gs_in[0].gTex;
	vec3 f_color = floor(255.0*(texture(texels,curr_tex).rgb)+0.5);
	
	vec3 last_tex;
	vec3 last_color;
	vec3 delta_color;
	gradient_t = 0;
	float aver_pa = 0.33;//x,y方向比例
	//左方像素的color
	last_tex = vec3(curr_tex.x-width_ratio,curr_tex.y,curr_tex.z);
	last_color = floor(255.0*(texture(texels,last_tex).rgb)+0.5);
	
	delta_color = abs(last_color-f_color);
	gradient_t += aver_pa*(delta_color.x + delta_color.y + delta_color.z);
	
	//上方像素的color
	last_tex = vec3(curr_tex.x,curr_tex.y-height_ratio,curr_tex.z);
	last_color = floor(255.0*(texture(texels,last_tex).rgb)+0.5);
	
	delta_color = abs(last_color-f_color);
	gradient_t += aver_pa*(delta_color.x + delta_color.y + delta_color.z);
	
	//上一帧的color
	last_tex = vec3(curr_tex.x,curr_tex.y,curr_tex.z-depth_ratio);
	last_color = floor(255.0*(texture(texels,last_tex).rgb)+0.5);
	delta_color = abs(last_color-f_color);
	gradient_t += aver_pa*(delta_color.x + delta_color.y + delta_color.z);
	
	EmitVertex();
	EndPrimitive();
}