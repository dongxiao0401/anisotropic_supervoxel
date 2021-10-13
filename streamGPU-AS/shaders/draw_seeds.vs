#version 430

layout(location = 0) in vec3 aPos;
layout(location = 1) in float seedId;
layout(location = 2) in vec3 seedMatrixFirst3;
layout(location = 3) in vec3 seedMatrixLast3;

uniform sampler3D image;
uniform float width_ratio;
uniform float height_ratio;
uniform float depth_ratio;

out VS_OUT {
	vec4 gSeedPosId;
	vec3 gSeedColor;
	vec3 gSeedMF3;
	vec3 gSeedML3;
} vs_out;

void main()
{
	vs_out.gSeedPosId = vec4(aPos,seedId);
	vec3 tex = vec3(aPos.x*width_ratio,aPos.y*height_ratio,aPos.z*depth_ratio);
	vs_out.gSeedColor = texture(image,tex).rgb;
	
	vs_out.gSeedMF3 = seedMatrixFirst3;
	vs_out.gSeedML3 = seedMatrixLast3;
	
	vec2 openglPos;
	
	openglPos.x = 2*aPos.x*width_ratio-1;
	openglPos.y = 2*aPos.y*height_ratio-1;
	
	gl_Position = vec4(openglPos,0.0,1.0);
}