#version 430
layout(location = 0) in vec3 aPos;
layout(location = 1) in float seedId;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec3 seedMatrixFirst3;
layout(location = 4) in vec3 seedMatrixLast3;


uniform float width_ratio;
uniform float height_ratio;




out VS_OUT {
	vec4 gSeedPosId;
	vec3 gSeedColor;
	vec3 gSeedMF3;
	vec3 gSeedML3;
} vs_out;


void main()
{
	vs_out.gSeedPosId = vec4(aPos,seedId);
	vs_out.gSeedColor = aColor;
	vs_out.gSeedMF3 = seedMatrixFirst3;
	vs_out.gSeedML3 = seedMatrixLast3;
	
	vec2 target;
	target.x = 2* aPos.x * width_ratio - 1;
	target.y = 2* aPos.y * height_ratio - 1;
	gl_Position = vec4(target,0.0,1.0);
	
}