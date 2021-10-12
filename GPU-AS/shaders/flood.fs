#version 430
layout(location=0) out vec4 FragColor; 


in vec4 f_color;
void main()
{
	
	FragColor = f_color;
}