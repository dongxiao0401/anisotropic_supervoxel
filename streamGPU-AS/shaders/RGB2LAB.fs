#version 430
layout(location=0) out vec3 FragColor; 


in vec3 f_color;
void main()
{
	//vec3 RGB = floor(f_color*255.0f+0.5f);//这里是因为把归一化了的恢复到0-255
	//FragColor = RGB;//实验观察，3d视频上，RGB分的比LAB更好

	vec3 RGB = f_color;//这里就用归一化了的值，下面RGB/255就可以省略了

	vec3 XYZ = vec3(0.0, 0.0, 0.0);
	vec3 temp = vec3(0.0, 0.0, 0.0);
	//vec3 paraX = vec3(0.4124564,0.3575761,0.1804375);//from SLIC
	//vec3 paraY = vec3(0.2126729,0.7151522,0.0721750);
	//vec3 paraZ = vec3(0.0193339,0.1191920,0.9503041);

	vec3 paraX = vec3(0.412453, 0.357580, 0.180423);//from LSC
	vec3 paraY = vec3(0.212671, 0.715160, 0.072169);
	vec3 paraZ = vec3(0.019334, 0.119193, 0.950227);

	//------------------------
	// sRGB to XYZ conversion
	//------------------------
	//RGB = RGB/255.0;
	if (RGB.r <= 0.04045)
		temp.r = RGB.r / 12.92;
	else
		temp.r = pow((RGB.r + 0.055) / 1.055, 2.4);

	if (RGB.g <= 0.04045)
		temp.g = RGB.g / 12.92;
	else
		temp.g = pow((RGB.g + 0.055) / 1.055, 2.4);

	if (RGB.b <= 0.04045)
		temp.b = RGB.b / 12.92;
	else
		temp.b = pow((RGB.b + 0.055) / 1.055, 2.4);

	XYZ.x = dot(temp, paraX);
	XYZ.y = dot(temp, paraY);
	XYZ.z = dot(temp, paraZ);


	//------------------------
	// XYZ to LAB conversion
	//------------------------
	float epsilon = 0.008856;	//actual CIE standard
	float kappa = 903.3;	//actual CIE standard
							//D50 0.96422,1.0,0.82521
							//vec3 ref_= vec3(0.96422,1.0,0.82521);
	vec3 ref_ = vec3(0.950456, 1.0, 1.088754);	//D65,reference white
	vec3 r_ = vec3(XYZ.x / ref_.x, XYZ.y / ref_.y, XYZ.z / ref_.z);
	vec3 f_ = vec3(0.0, 0.0, 0.0);
	if (r_.x>epsilon)
		f_.x = pow(r_.x, 1.0 / 3.0);
	else
		f_.x = (kappa*r_.x + 16.0) / 116.0;

	if (r_.y>epsilon)
		f_.y = pow(r_.y, 1.0 / 3.0);
	else
		f_.y = (kappa*r_.y + 16.0) / 116.0;

	if (r_.z>epsilon)
		f_.z = pow(r_.z, 1.0 / 3.0);
	else
		f_.z = (kappa*r_.z + 16.0) / 116.0;

	vec3 lab_val = vec3(116.0*f_.y - 16.0, 500.0*(f_.x - f_.y), 200.0*(f_.y - f_.z));
	//下面是把0≤L≤100, −127≤a≤127, −127≤b≤127转成L←L∗255/100,a←a+128,b←b+128
	lab_val.x = lab_val.x*2.55;
	lab_val.y = lab_val.y + 128;
	lab_val.z = lab_val.z + 128;
	lab_val = floor(lab_val + 0.5);

	FragColor = lab_val;


}
