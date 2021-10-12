#version 430

layout(location = 0) in ivec3 aPos;


uniform float width_ratio;//2*x/w-1,width_ratio=1/w
uniform float height_ratio;
uniform float depth_ratio;
out VS_OUT {
	int  gLayer;
	vec3 gTex;//texture坐标，texture()函数使用
} vs_out;



void main()
{
	vec3 opengl_pos = vec3(aPos.x+0.5,aPos.y+0.5,aPos.z+0.5);
	
	vs_out.gTex =  vec3(opengl_pos.x*width_ratio,opengl_pos.y*height_ratio,opengl_pos.z*depth_ratio);
	
	opengl_pos.x = 2*(vs_out.gTex.x)-1;
	opengl_pos.y = 2*(vs_out.gTex.y)-1;
	
	gl_Position = vec4(opengl_pos.xy,0.0,1.0);
	vs_out.gLayer = aPos.z;
}