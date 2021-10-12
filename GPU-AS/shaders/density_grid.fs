#version 430
layout(location=0) out vec4 FragPos; 
//layout(location=1) out vec4 FragColor; 

//in vec3 f_pos;
//in vec3 f_color;
in float f_grad;
void main()
{
	FragPos = vec4(f_grad,1,0,0);
	//FragColor = vec4(f_color,1.0f);
}