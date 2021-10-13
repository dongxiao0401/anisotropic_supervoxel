#version 430
layout(points )in;
layout(points ,max_vertices = 1 )out;

uniform sampler3D texels;
uniform sampler3D sColorImage;
uniform sampler3D seedMatrixFirst3;
uniform sampler3D seedMatrixLast3;
uniform sampler3D image;
uniform float width_ratio;//2*x/w-1,width_ratio=1/w
uniform float height_ratio;
uniform float depth_ratio;
uniform float step;
uniform float ratio;


in VS_OUT {
	vec3 gPos;
	vec3 gTex;
} gs_in[];

out vec4 f_color;


void main(){
	
	gl_Position=gl_in[0].gl_Position;
	gl_Layer = int(gs_in[0].gPos.z);
	vec3 curr_xyz = gs_in[0].gPos;
	
	
	//8邻居
	float s_w_r = step*width_ratio;
	float s_h_r = step*height_ratio;
	float off_set[18] = 
	{-1.0*s_w_r,-1.0*s_h_r,
	0,-1.0*s_h_r,
	s_w_r,-1.0*s_h_r,
	-1.0*s_w_r,0,
	0,0,
	s_w_r,0,
	-1.0*s_w_r,s_h_r,
	0,s_h_r,
	s_w_r,s_h_r
	};
	

	vec3 whd_ratio = vec3(width_ratio,height_ratio,depth_ratio);
	vec3 this_slice = gs_in[0].gTex;
	vec3 curr_color = texture(image,this_slice).rgb;
	
	vec3 neighbors;
	vec4 neighbor_seed;
	vec3 n_seed_xyz;
	vec3 delta;
	vec3 seed_tex;
	vec3 seed_color;
	vec3 seed_matrix_f3;
	vec3 seed_matrix_l3;
	f_color = vec4(-1.0f,-1.0f,-1.0f,-1.0f);
	
	float min_dist=100000000;
	float curr_pos_dist=0;
	//float curr_depth_dist=0;
	float curr_color_dist=0;
	//float curr_illu_dist=0;
	float curr_dist;
	
	
	//在该slice上的9个neighbors
	int index=0;
	for(int i=0;i<9;i++)
	{
		neighbors = this_slice+ vec3(off_set[index],off_set[index+1],0);
		neighbor_seed = texture(texels,neighbors);
		if(neighbor_seed.x>-1)
		{
			n_seed_xyz = neighbor_seed.xyz;
			delta = curr_xyz-n_seed_xyz;
			seed_tex = n_seed_xyz*whd_ratio;
			seed_color = texture(sColorImage,seed_tex).rgb;
			seed_matrix_f3 = texture(seedMatrixFirst3,seed_tex).rgb;
			seed_matrix_l3 = texture(seedMatrixLast3,seed_tex).rgb;
			curr_pos_dist = dot(vec3(delta.x*delta.x,2*delta.x*delta.y,2*delta.x*delta.z),seed_matrix_f3)+dot(vec3(delta.y*delta.y,2*delta.y*delta.z,delta.z*delta.z),seed_matrix_l3);
			//n_seed_xyz.z=curr_xyz.z;
			//curr_pos_dist = dot(delta,delta);
			//curr_depth_dist = (curr_xyz.z-n_seed_xyz.z)*(curr_xyz.z-n_seed_xyz.z);
			curr_color_dist = dot((curr_color-seed_color),(curr_color-seed_color));
			if(curr_pos_dist<0)
			{
				curr_pos_dist=0;
			}
			curr_dist = ratio*(curr_pos_dist)+curr_color_dist;
			if(curr_dist<min_dist)
			{
				min_dist = curr_dist;
				f_color = neighbor_seed;
			}
			
		}
		index +=2;
	}


	this_slice.z = (curr_xyz.z-step)*depth_ratio;
	index = 0;
	for(int i=0;i<9;i++)
	{
		neighbors = this_slice+ vec3(off_set[index],off_set[index+1],0);
		neighbor_seed = texture(texels,neighbors);
		if(neighbor_seed.x>-1)
		{
			n_seed_xyz = neighbor_seed.xyz;
			delta = curr_xyz-n_seed_xyz;
			seed_tex = n_seed_xyz*whd_ratio;
			seed_color = texture(sColorImage,seed_tex).rgb;
			seed_matrix_f3 = texture(seedMatrixFirst3,seed_tex).rgb;
			seed_matrix_l3 = texture(seedMatrixLast3,seed_tex).rgb;
			curr_pos_dist = dot(vec3(delta.x*delta.x,2*delta.x*delta.y,2*delta.x*delta.z),seed_matrix_f3)+dot(vec3(delta.y*delta.y,2*delta.y*delta.z,delta.z*delta.z),seed_matrix_l3);
			//curr_pos_dist = dot(delta,delta);
			//curr_depth_dist =(curr_xyz.z-n_seed_xyz.z)*(curr_xyz.z-n_seed_xyz.z);
			curr_color_dist = dot((curr_color-seed_color),(curr_color-seed_color));
			if(curr_pos_dist<0)
			{
				curr_pos_dist=0;
			}
			curr_dist = ratio*(curr_pos_dist)+curr_color_dist;
			if(curr_dist<min_dist)
			{
				min_dist = curr_dist;
				f_color = neighbor_seed;
			}
			
		}
		index +=2;
	}
	

	this_slice.z = (curr_xyz.z+step)*depth_ratio;
	index = 0;
	for(int i=0;i<9;i++)
	{
		neighbors = this_slice+ vec3(off_set[index],off_set[index+1],0);
		neighbor_seed = texture(texels,neighbors);
		if(neighbor_seed.x>-1)
		{
			n_seed_xyz = neighbor_seed.xyz;
			delta = curr_xyz-n_seed_xyz;
			seed_tex = n_seed_xyz*whd_ratio;
			seed_color = texture(sColorImage,seed_tex).rgb;
			seed_matrix_f3 = texture(seedMatrixFirst3,seed_tex).rgb;
			seed_matrix_l3 = texture(seedMatrixLast3,seed_tex).rgb;
			curr_pos_dist = dot(vec3(delta.x*delta.x,2*delta.x*delta.y,2*delta.x*delta.z),seed_matrix_f3)+dot(vec3(delta.y*delta.y,2*delta.y*delta.z,delta.z*delta.z),seed_matrix_l3);
			//curr_pos_dist = dot(delta,delta);
			//curr_depth_dist = (curr_xyz.z-n_seed_xyz.z)*(curr_xyz.z-n_seed_xyz.z);
			curr_color_dist = dot((curr_color-seed_color),(curr_color-seed_color));
			//curr_illu_dist = (curr_color.x-seed_color.x)*(curr_color.x-seed_color.x);
			//curr_dist = ratio*curr_pos_dist+curr_color_dist;
			if(curr_pos_dist<0)
			{
				curr_pos_dist=0;
			}
			curr_dist = ratio*(curr_pos_dist)+curr_color_dist;
			if(curr_dist<min_dist)
			{
				min_dist = curr_dist;
				f_color = neighbor_seed;
			}
			
		}
		index +=2;
	}
	
	


	//f_color=5;
	EmitVertex();
	EndPrimitive();
	
}