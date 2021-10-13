#version 430
layout(location=0) out vec4 FragPos; 

in float f_grad;
void main()
{
	FragPos = vec4(f_grad,1,0,0);
}