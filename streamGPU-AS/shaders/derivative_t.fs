#version 430
layout(location=0) out vec3 FragColor; 


in float gradient_t;

void main()
{
	FragColor = vec3(gradient_t,-1,-1);
}
