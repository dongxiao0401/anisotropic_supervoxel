#version 430
layout(location=0) out int FragColor; 


flat in int f_label_i;

void main()
{
	FragColor = f_label_i;
}