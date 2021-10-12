#version 430
layout(location = 0) in ivec3 aPos;


uniform sampler3D grid;
uniform sampler3D gradient;
uniform float width_ratio;
uniform float height_ratio;
uniform float depth_ratio;


out VS_OUT {
	int gLayer;
	float p_grad;
} vs_out;

void main()
{
	vec3 curr_tex = vec3(aPos.x+0.5,aPos.y+0.5,aPos.z+0.5);
	curr_tex.x *= width_ratio;
	curr_tex.y *= height_ratio;
	curr_tex.z *= depth_ratio;
	float targetId = texture(grid,curr_tex).r;//rgba或者xyzw
	vs_out.p_grad = texture(gradient,curr_tex).r;
	vec3 target;
	//target.z = floor(targetId*width_ratio*height_ratio);//如果id超出了一帧图片的大小，则将该seed写到下一帧
	target.z=0;
	target.y = floor(targetId*width_ratio);
	target.x = floor(targetId-target.y/width_ratio);
	target+=0.5;

	target.x = target.x*width_ratio;
	target.y = target.y*height_ratio;
	vec2 opengl_xy = 2*(target.xy)-1;
	
	
	//vec2 opengl_xy = 2*(curr_tex.xy)-1;
	
	gl_Position = vec4(opengl_xy,0.0,1.0);
	//vs_out.gLayer = aPos.z;
	
	vs_out.gLayer = int(target.z);
	
}