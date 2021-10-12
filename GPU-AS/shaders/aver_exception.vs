#version 430
layout(location = 0) in vec3 texPos;
layout(location = 1) in float seedId;


uniform sampler3D sum_pos;
uniform sampler3D sum_color;
uniform sampler3D image;
uniform float width;
uniform float height;
uniform float depth;




out VS_OUT {
	int gLayer;
	vec4 gCentroidPos;
	vec3 gCentroidColor;
} vs_out;

void main()
{
	vec4 center = texture(sum_pos,texPos);//rgba或者xyzw
	if(int(center.w)==0)
	{
		vec3 neighbor_tex;
		vec4 neighbor_pos,neighbor_pos2;
		neighbor_tex.z = 0;
		neighbor_tex.y = floor((seedId-1)/width);
		neighbor_tex.x = floor(seedId-1-neighbor_tex.y*width);
		neighbor_tex += 0.5;
		neighbor_tex.x /= width;
		neighbor_tex.y /= height;
		neighbor_tex.z /= depth;
		neighbor_pos = texture(sum_pos,neighbor_tex);
		if(int(neighbor_pos.w)!=0)
		{
			neighbor_pos.xyz /= neighbor_pos.w;
		}
		

		neighbor_tex.z = 0;
		neighbor_tex.y = floor((seedId+1)/width);
		neighbor_tex.x = floor(seedId+1-neighbor_tex.y*width);
		neighbor_tex += 0.5;
		neighbor_tex.x /= width;
		neighbor_tex.y /= height;
		neighbor_tex.z /= depth;
		neighbor_pos2 = texture(sum_pos,neighbor_tex);
		if(int(neighbor_pos2.w)!=0)
		{
			neighbor_pos2.xyz /= neighbor_pos2.w;
		}
		center.xyz = floor(0.5*(neighbor_pos.xyz+neighbor_pos2.xyz))+0.5;
		vec3 center_tex;
		center_tex.x = center.x/width;
		center_tex.y = center.y/height;
		center_tex.z = center.z/depth;
		vs_out.gCentroidColor = texture(image,center_tex).rgb;
		center.w = seedId;
	}
	else
	{
	center.xyz /= center.w;
	center.xyz = floor(center.xyz)+0.5;
	vs_out.gCentroidColor = texture(sum_color,texPos).rgb;
	vs_out.gCentroidColor /= center.w;
	center.w = seedId;
	}
	
	
	
	
	
	
	vec2 opengl_xy = 2*(texPos.xy)-1;

	gl_Position = vec4(opengl_xy,0.0,1.0);
	vs_out.gLayer = int(texPos.z*depth);
	vs_out.gCentroidPos = center;
}