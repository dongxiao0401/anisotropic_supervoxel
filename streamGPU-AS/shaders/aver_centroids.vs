#version 430
layout(location = 0) in vec3 texPos;
layout(location = 1) in float seedId;


uniform sampler3D texels;
uniform float depth;




out VS_OUT {
	int gLayer;
	vec4 gCentroid;
} vs_out;

void main()
{
	vec4 center = texture(texels,texPos);//rgba或者xyzw
	center.xyz /= center.w;
	center.xyz = floor(center.xyz)+0.5;
	center.w = seedId;
	
	
	
	vec2 opengl_xy = 2*(texPos.xy)-1;

	gl_Position = vec4(opengl_xy,0.0,1.0);
	vs_out.gLayer = int(texPos.z*depth);
	vs_out.gCentroid = center;
}