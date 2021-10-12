#version 430
layout(location=0) out int FragColor; 


flat in int f_label_i;
//in float f_label;

void main()
{
	//FragColor = int(f_label);
	FragColor = f_label_i;
}