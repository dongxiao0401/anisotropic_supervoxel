#version 430
layout(location=0) out vec3 FragColor; 


//in vec3 f_color;
in float gradient_t;

void main()
{
	//FragColor = f_color;
	FragColor = vec3(gradient_t,-1,-1);
}
