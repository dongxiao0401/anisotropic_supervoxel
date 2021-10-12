#version 430
layout(location=0) out vec4 FragPos; 
layout(location=1) out vec4 FragColor; 

in vec3 f_pos;
in vec3 f_color;
void main()
{
	FragPos = vec4(f_pos,1.0f);
	FragColor = vec4(f_color,1.0f);
}