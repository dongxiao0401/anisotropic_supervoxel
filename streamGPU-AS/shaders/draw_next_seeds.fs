#version 430
layout(location=0) out vec4 FragPos; 
layout(location=1) out vec4 FragColor; 
layout(location=2) out vec4 FragMatrixFirst3; 
layout(location=3) out vec4 FragMatrixLast3; 

in vec4 f_pos;
in vec3 f_color;
in vec3 f_MF3;
in vec3 f_ML3;
void main()
{
	FragPos = f_pos;
	FragColor = vec4(f_color,f_pos.w);//vec4(f_color,0);
	FragMatrixFirst3 = vec4(f_MF3,f_pos.w);
	FragMatrixLast3 = vec4(f_ML3,f_pos.w);
}