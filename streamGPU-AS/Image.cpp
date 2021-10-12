#include "Image.h"

Image::Image()
{
	width = 100;
	height = 100;
	current_buffer = 1;

}

Image::~Image()
{

}
void Image::unload()
{
	glDeleteTextures(4, texture_image);
	glDeleteFramebuffers(1, &frameBuffer_image);
	glDeleteTextures(5, texture_flood);
	glDeleteFramebuffers(1, &frameBuffer_flood);

}



void Image::initialization_grid(int& ac_seeds_num_)
{
	//init grid seeds
	//-----------------------------------
	double voxel_size = texel_num / seeds_num;
	s = int(floor(pow(voxel_size, 1.0 / 3)));
	seeds_lsc.clear();
	seeds_lsc_adjust.clear();
	seeds_lsc.reserve(seeds_num * 4);
	seeds_lsc_adjust.reserve(seeds_num * 4);
	int  stepD,stepH, stepW;
	dNum = pow(float(seeds_num*depth*depth / width / height), 1.0 / 3.0);
	if (dNum == 0)
		dNum = 1;
	/*if (dNum > 2)
		dNum = dNum - 1;*/
	int frame_seeds_num = seeds_num / dNum;
	wNum = sqrt(float(frame_seeds_num*width / height));
	hNum = frame_seeds_num / wNum;
	ac_seeds_num = dNum * hNum*wNum;
	ac_seeds_num_ = ac_seeds_num;
	stepD = depth / dNum;
	stepH = height / hNum;
	stepW = width / wNum;
	max_s = stepD;
	max_s = stepW > max_s ? stepW : max_s;
	max_s = stepH > max_s ? stepH : max_s;
	

	int d_remain = depth - stepD * dNum;
	int h_remain = height - stepH * hNum;
	int w_remain = width - stepW * wNum;
	int td = 1, th = 1,tw=1;
	td = (d_remain < 1) ? 0 : 1;
	th = (h_remain < 1) ? 0 : 1;
	tw = (w_remain < 1) ? 0 : 1;
	int count = 0;
	int seedx, seedy, seedz;
	for (int k = 0; k < dNum; k++)
	{
		th = 1;
		for (int i = 0; i < hNum; i++)
		{
			tw = 1;
			for (int j = 0; j < wNum; j++)
			{
				seedx = j * stepW + 0.5*stepW + tw;
				seedy = i * stepH + 0.5*stepH + th;
				seedz = k * stepD + 0.5*stepD + td;
				seedx = (seedx > width - 1) ? (width - 1) : seedx;
				seedy = (seedy > height - 1) ? (height -1) : seedy;
				seedz = (seedz > depth - 1) ? (depth - 1) : seedz;
				if (tw < w_remain)
				tw++;

				seeds_lsc.push_back(seedx+0.5);
				seeds_lsc.push_back(seedy+0.5);
				seeds_lsc.push_back(seedz+0.5);
				seeds_lsc.push_back(count);

				seeds_lsc_adjust.push_back(seedx + 0.5);
				seeds_lsc_adjust.push_back(seedy + 0.5);
				seeds_lsc_adjust.push_back(seedz + 0.5);
				seeds_lsc_adjust.push_back(count);

				count++;
				//cout << seedx << "," << seedy << "," << seedz << endl;

			}
			
			if (th < h_remain)
				th++;
		}

		if (td < d_remain)
			td++;
	}
	cout << "placed num:" << count << endl;


	
	glGenVertexArrays(1, &VAO_s);
	glGenBuffers(1, &VBO_s);
	glBindVertexArray(VAO_s);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_s);
	glBufferData(GL_ARRAY_BUFFER, seeds_lsc.size() * sizeof(float), &seeds_lsc[0], GL_STATIC_DRAW);//sizeof(pos)只能当pos时数组时才能这样用。比如float pos[]={1,1,1};,这样的话sizeof(pos)=3*sizeof(float)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//-----------------------------------


}

void Image::get_ac_seeds(vector<float>& seeds_all)
{
	seeds_all.clear();
	seeds_all = seeds_lsc_adjust;
	/*cout << "!!!" << endl;
	for (int i = 0;i < ac_seeds_num; i++)
	{
		cout << seeds_lsc_adjust[i * 4] << "," << seeds_lsc_adjust[i * 4+1] << ","<<seeds_lsc_adjust[i * 4+2] << ","<<seeds_lsc_adjust[i * 4+3] << endl;
	}*/
}



bool Image::init_flood_texture()
{
	frameBuffer_flood = 0;
	buffers_flood[0] = GL_COLOR_ATTACHMENT0;
	buffers_flood[1] = GL_COLOR_ATTACHMENT1;
	buffers_flood[2] = GL_COLOR_ATTACHMENT2;
	buffers_flood[3] = GL_COLOR_ATTACHMENT3;
	buffers_flood[4] = GL_COLOR_ATTACHMENT4;
	glGenFramebuffers(1, &frameBuffer_flood);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_flood);


	for (unsigned int i = 0; i < 5; i++)
	{
		glGenTextures(1, texture_flood + i);
		glBindTexture(GL_TEXTURE_3D, texture_flood[i]);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, width, height, depth, 0, GL_RGBA,
			GL_FLOAT, NULL);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, texture_flood[i], 0);
	}


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("something wrong about framebuffer!");
		return false;
	}
}
bool Image::init_image_texture(unsigned char* data)
{
	frameBuffer_image = 0;
	buffers_image[0] = GL_COLOR_ATTACHMENT0;
	buffers_image[1] = GL_COLOR_ATTACHMENT1;
	buffers_image[2] = GL_COLOR_ATTACHMENT2;
	buffers_image[3] = GL_COLOR_ATTACHMENT3;
	glGenFramebuffers(1, &frameBuffer_image);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_image);
	glGenTextures(1, texture_image + 0);
	glBindTexture(GL_TEXTURE_3D, texture_image[0]);
	if (data)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, width, height, depth, 0, GL_RGB,
			GL_UNSIGNED_BYTE, data);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_image[0], 0);
	}
	else
	{
		std::cout << "Failed to load 3d texture!" << std::endl;
	}

	/*glBindTexture(GL_TEXTURE_3D, texture_image[0]);
	print_buffer3(0);*/

	for (unsigned int i = 1; i < 2; ++i)
	{
		glGenTextures(1, texture_image + i);
		glBindTexture(GL_TEXTURE_3D, texture_image[i]);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, width, height, depth, 0, GL_RGB,
			GL_HALF_FLOAT, NULL);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, texture_image[i], 0);
	}

	glGenTextures(1, texture_image + 2);
	glBindTexture(GL_TEXTURE_3D, texture_image[2]);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, width, height, depth, 0, GL_RED,
		GL_HALF_FLOAT, NULL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 2, texture_image[2], 0);

	glGenTextures(1, texture_image + 3);
	glBindTexture(GL_TEXTURE_3D, texture_image[3]);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R16I, width, height, depth, 0, GL_RED_INTEGER,
		GL_INT, NULL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 3, texture_image[3], 0);


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("something wrong about image framebuffer!");
		return false;
	}


	/*glBindTexture(GL_TEXTURE_3D, texture_image[0]);
	output_3d();*/

}

void Image::output_3d()
{
	unsigned char* res1 = new unsigned char[texel_num * 3];//GL_RGB,unsigned char
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RGB, GL_UNSIGNED_BYTE, res1);
	string txtname = "rgb_3d.txt";
	ofstream f_debug1(txtname);
	int pi = 0;
	for (int i = 0; i < depth * height; i++) {
		for (int j = 0; j < width; j++) {
			f_debug1 << std::left << std::setw(4) << int(res1[pi++]) << " ";
			f_debug1 << std::left << std::setw(4) << int(res1[pi++]) << " ";
			f_debug1 << std::left << std::setw(4) << int(res1[pi++]) << " ";
		}
		f_debug1 << endl;
		if ((i + 1) % height == 0)
			f_debug1 << endl << endl;
	}
	f_debug1.close();
}

void Image::RGB2LAB(Shader& rgb2lab)
{
	rgb2lab.use();
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_image);
	glDrawBuffers(1, buffers_image + 1);
	glClearColor(3.0f, -1.0f, -1.0f, -1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, texture_image[0]);
	rgb2lab.setInt("texels", 0);
	rgb2lab.setFloat("width_ratio", width_ratio);
	rgb2lab.setFloat("height_ratio", height_ratio);
	rgb2lab.setFloat("depth_ratio", depth_ratio);

	glBindVertexArray(VAO_all_t);
	glDrawArrays(GL_POINTS, 0, texel_num);

	//glBindTexture(GL_TEXTURE_3D, texture_image[1]);
	//print_buffer3(1);
}
bool Image::init_gpu(unsigned char* data, int width_, int height_, int depth_, int seeds_num_, int Lloyd_max_iter_)
{
	width = width_;
	height = height_;
	depth = depth_;
	width_f = float(width);
	height_f = float(height);
	depth_f = float(depth);
	width_ratio = 1.0 / width_f;
	height_ratio = 1.0 / height_f;
	depth_ratio = 1.0 / depth_f;
	seeds_num = seeds_num_;
	slice_num = width * height;
	texel_num = slice_num * depth;
	Lloyd_max_iter = Lloyd_max_iter_;


	init_image_texture(data);
	init_flood_texture();

	//init_vaos();
	//init_variables();
	glEnable(GL_TEXTURE_3D);
	glViewport(0, 0, width, height);
	glDisable(GL_DEPTH_TEST);



	return true;
}

/*
Adjust initialization based on gradient
*/
void Image::initialization_adjust(Shader& derivative_t,Shader& density_grid)
{
	vector<int> seed_range3d;
	seed_range3d.reserve(ac_seeds_num * 6);
	vector<int> seed_range1d;
	seed_range1d.reserve(ac_seeds_num * 2);
	float curr_x = 0, curr_y = 0, curr_d = 0;
	float ng = 0;
	int curr_sindex = 0, ng_sindex = 0, ng2_sindex = 0;
	for (int t = 0; t < dNum; t++)
	{
		for (int j = 0; j < hNum; j++)
		{
			for (int i = 0; i < wNum; i++)
			{
				curr_x = seeds_lsc[curr_sindex * 4];
				curr_y = seeds_lsc[curr_sindex * 4 + 1];
				curr_d = seeds_lsc[curr_sindex * 4 + 2];

				//left
				if (i > 0)
				{
					ng_sindex = curr_sindex - 1;
					ng = seeds_lsc[ng_sindex * 4];
					ng = floor(0.5*(ng + curr_x));
					seed_range3d.push_back(int(ng));
					seed_range1d.push_back(int(ng));
				}
				else
				{
					seed_range3d.push_back(0);
					seed_range1d.push_back(0);
				}
				//right
				if (i < wNum - 1)
				{
					ng_sindex = curr_sindex + 1;
					ng = seeds_lsc[ng_sindex * 4];
					ng = floor(0.5*(ng + curr_x));
					seed_range3d.push_back(int(ng));
					seed_range1d.push_back(int(ng));
				}
				else
				{
					seed_range3d.push_back(width - 1);
					seed_range1d.push_back(width - 1);
				}
				//up
				if (j > 0)
				{
					ng_sindex = curr_sindex - wNum;
					ng = seeds_lsc[ng_sindex * 4 + 1];
					ng = floor(0.5*(ng + curr_y));
					seed_range3d.push_back(int(ng));
				}
				else
					seed_range3d.push_back(0);
				//down
				if (j <hNum - 1)
				{
					ng_sindex = curr_sindex + wNum;
					ng = seeds_lsc[ng_sindex * 4 + 1];
					ng = floor(0.5*(ng + curr_y));
					seed_range3d.push_back(int(ng));
				}
				else
					seed_range3d.push_back(height - 1);
				//front
				if (t > 0)
				{
					ng_sindex = curr_sindex - wNum * hNum;
					ng = seeds_lsc[ng_sindex * 4 + 2];
					ng = floor(0.5*(ng + curr_d));
					seed_range3d.push_back(int(ng));
				}
				else
					seed_range3d.push_back(0);
				//back
				if (t < dNum - 1)
				{
					ng_sindex = curr_sindex + wNum * hNum;
					ng = seeds_lsc[ng_sindex * 4 + 2];
					ng = floor(0.5*(ng + curr_d));
					seed_range3d.push_back(int(ng));
				}
				else
					seed_range3d.push_back(depth - 1);


				curr_sindex++;
			}
		}
	}

	float* grid = new float[texel_num];
	int d1, d2, y1, y2, x1, x2;
	int index = 0;
	int voxel_index = 0;
	for (int k = 0; k < ac_seeds_num; k++)
	{
		x1 = seed_range3d[index++];
		x2 = seed_range3d[index++];
		y1 = seed_range3d[index++];
		y2 = seed_range3d[index++];
		d1 = seed_range3d[index++];
		d2 = seed_range3d[index++];
		for (int t = d1; t <= d2; t++)
		{
			for (int j = y1; j <= y2; j++)
			{
				for (int i = x1; i <= x2; i++)
				{
					voxel_index = t * slice_num + j * width + i;
					grid[voxel_index] = k;
				}
			}
		}
	}


	/*string txtname = "grid.txt";
	ofstream f_debug1(txtname);
	int pi = 0;
	for (int i = 0; i < depth * height; i++) {
		for (int j = 0; j < width; j++) {
			f_debug1 << std::left << std::setw(4) << int(grid[pi++]) << " ";
		}
		f_debug1 << endl;
		if ((i + 1) % height == 0)
			f_debug1 << endl << endl;
	}
	f_debug1.close();*/



	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_image);
	glBindTexture(GL_TEXTURE_3D, texture_image[2]);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, width, height, depth, 0, GL_RED,
		GL_FLOAT, grid);


	if (grid) delete[] grid;


	//1.Partial derivative in x,y and t dimensions
	
	derivative_t.use();
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_image);
	glDrawBuffers(1, buffers_image + 1);
	glClearColor(3.0f, -1.0f, -1.0f, -1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, texture_image[0]);
	derivative_t.setInt("texels", 0);
	derivative_t.setFloat("width_ratio", width_ratio);
	derivative_t.setFloat("height_ratio", height_ratio);
	derivative_t.setFloat("depth_ratio", depth_ratio);
	glBindVertexArray(VAO_all_t);
	glDrawArrays(GL_POINTS, 0, texel_num);

	/*glBindTexture(GL_TEXTURE_3D, texture_image[1]);
	print_buffer3(1);*/


	
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);
	density_grid.use();
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_flood);
	glDrawBuffers(1, buffers_flood + 2);
	glClearColor(3.0f, -1.0f, -1.0f, -1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, texture_image[2]);
	density_grid.setInt("grid", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, texture_image[1]);
	density_grid.setInt("gradient", 1);
	density_grid.setFloat("width_ratio", width_ratio);
	density_grid.setFloat("height_ratio", height_ratio);
	density_grid.setFloat("depth_ratio", depth_ratio);
	glBindVertexArray(VAO_all_t);
	glDrawArrays(GL_POINTS, 0, texel_num);
	glDisable(GL_BLEND);


	glBindTexture(GL_TEXTURE_3D, texture_flood[2]);
	int read_height = ac_seeds_num / width + 1;
	float* res = new float[4 * read_height*width];
	GLuint fboId = 0;
	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
	glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		texture_flood[2], 0, 0);
	glReadPixels(0,0,width,read_height,GL_RGBA,GL_FLOAT,res);
	

	vector<float> seed_density;
	int number_one_layer = ac_seeds_num;
	seed_density.reserve(number_one_layer);
	for (int i = 0; i < number_one_layer; i++)
	{
		seed_density.push_back(0.001*res[i * 4]);	
	}
	vector<float> seed_length;
	seed_length.reserve(number_one_layer);
	for (int i = 0; i < number_one_layer; i++)
	{
		if (i == 0)
		{
			seed_length.push_back(seed_density[i]);
		}
		else
		{
			seed_length.push_back(seed_density[i] + seed_length[i - 1]);
		}

	}
	float interval = seed_length[number_one_layer - 1] / (number_one_layer+1);
	vector<float> seed_in;
	seed_in.reserve(number_one_layer);
	for (int i = 0; i < number_one_layer; i++)
	{
		seed_in.push_back((i + 1)*interval);
	}


	seeds_lsc_adjust.clear();
	seeds_lsc_adjust.reserve(ac_seeds_num * 4);
	seeds_lsc_adjust.clear();
	float adjust_x = 0;
	int backforth = 1;
	int stepD = depth / dNum;
	float adjust_y = 0;

	int j1 = 0;
	for (int i = 0; i < number_one_layer; )
	{
		if (seed_in[i] < seed_length[j1])
		{
			if (j1 == 0)
			{
				adjust_x = floor(seed_range1d[j1 * 2] + (seed_range1d[j1 * 2 + 1] - seed_range1d[j1 * 2])*(seed_in[i] / seed_length[j1])) + 0.5;
			}
			else
			{
				adjust_x = floor(seed_range1d[j1 * 2] + (seed_range1d[j1 * 2 + 1] - seed_range1d[j1 * 2])*((seed_in[i] - seed_length[j1 - 1]) / (seed_length[j1] - seed_length[j1 - 1]))) + 0.5;
			}

			adjust_y = seeds_lsc[j1 * 4 + 1];


			seeds_lsc_adjust.push_back(adjust_x);
			seeds_lsc_adjust.push_back(adjust_y);
			seeds_lsc_adjust.push_back(seeds_lsc[j1 * 4 + 2]);
			//cout << i<<":(" << adjust_x << "," << adjust_y << "," << seeds_lsc[j1 * 4 + 2] << ")" << endl;
			seeds_lsc_adjust.push_back(float(i));

			i++;
		}
		else
		{
			j1++;
			if (j1 == ac_seeds_num)
				break;
		}

	}

	
	if (res) delete[] res;
	seed_length.clear();
}

void Image::print_flow_mat(Mat& flow)
{
	string name = "flow_xy.txt";
	ofstream f_debug(name);
	int slice_index = 0;
	int row = flow.rows;
	int col = flow.cols*flow.channels();
	for (int i = 0; i < row; i++) {
		float* ptr = flow.ptr <float>(i);
		for (int j = 0; j < col; j++) {
			f_debug << std::left << std::setw(4) << int(ptr[j]) << " ";
		}
		f_debug << endl;
	}

	f_debug.close();
}


void Image::seed_matrix(vector<Mat>& flow_forward, vector<Mat>& flow_backward, vector<float>& seeds_lsc_adjust_)
{
	seeds_lsc_adjust.clear();
	seeds_lsc_adjust = seeds_lsc_adjust_;
	
	seeds_lsc.clear();
	seeds_lsc.reserve(ac_seeds_num * 10);//x,y,z,id,symmetric matrix 6 unknowns
	vector<float> curr_matrix;
	vector<float> self_matrix;
	int neg_seed_matrix_num = 0;
	int on_frame_index = 0;
	float seed_x = 0, seed_y = 0,seed_z=0;
	int index = 0;
	//index = 48 * 10;

	int ng_1ring_x[8] = { -1,0,1,-1,1,-1,0,1 };
	int ng_1ring_y[8] = { -1,-1,-1,0,0,1,1,1 };

	for (int i = 0; i < ac_seeds_num; i++)
	{
		seed_x = seeds_lsc_adjust[index++];
		seed_y = seeds_lsc_adjust[index++];
		seed_z = seeds_lsc_adjust[index++];
		on_frame_index = int(seed_z);
		index++;
		seeds_lsc.push_back(seed_x);
		seeds_lsc.push_back(seed_y);
		seeds_lsc.push_back(seed_z);
		seeds_lsc.push_back(float(i));
		seed_x = floor(seed_x); 
		seed_y = floor(seed_y);
		
		curr_matrix.clear();
		bool neg_eigen_value = matrix_slover(seed_x, seed_y, on_frame_index, flow_forward, flow_backward, curr_matrix);
		self_matrix.clear();
		self_matrix.reserve(6);
		self_matrix.assign(curr_matrix.begin(), curr_matrix.end());
		
		if (!neg_eigen_value)
		{
			for (int t = 0; t < 6; t++)
			{
				seeds_lsc.push_back(curr_matrix[t]);
			}
		}
		else
		{
			neg_seed_matrix_num++;
			int valid_ng_matrix_num = 0;
			vector<float> ng_matrix_sum;
			for (int t = 0; t < 6; t++)
			{
				ng_matrix_sum.push_back(0);
			}
			float seed_ng_x = 0, seed_ng_y = 0;
			for (int t = 0; t < 8; t++)
			{
				curr_matrix.clear();
				seed_ng_x = seed_x + ng_1ring_x[t];
				seed_ng_y = seed_y + ng_1ring_y[t];
				if (seed_ng_x >= 0 && seed_ng_x < width&&seed_ng_y >= 0 && seed_ng_y < height)
				{
					neg_eigen_value = matrix_slover(seed_ng_x, seed_ng_y, on_frame_index, flow_forward, flow_backward, curr_matrix);
					if (!neg_eigen_value)
					{
						valid_ng_matrix_num++;
						for (int t = 0; t < 6; t++)
						{
							ng_matrix_sum[t] = (ng_matrix_sum[t] + curr_matrix[t]);
						}
					}
				}
			}
			
			if (valid_ng_matrix_num == 0)
			{
				//cout << "1-ring neighbors no SPD matrix, search two-ring neighbors" << endl;
				int ng_2ring_x[16] = { -2,-1,0,1,2,-2,2,-2,2,-2,2,-2,-1,0,1,2 };
				int ng_2ring_y[16] = { -2,-2,-2,-2,-2,-1,-1,0,0,1,1,2,2,2,2,2 };
				for (int t = 0; t < 16; t++)
				{
					curr_matrix.clear();
					seed_ng_x = seed_x + ng_2ring_x[t];
					seed_ng_y = seed_y + ng_2ring_y[t];
					if (seed_ng_x >= 0 && seed_ng_x < width&&seed_ng_y >= 0 && seed_ng_y < height)
					{
						neg_eigen_value = matrix_slover(seed_ng_x, seed_ng_y, on_frame_index, flow_forward, flow_backward, curr_matrix);
						if (!neg_eigen_value)
						{
							valid_ng_matrix_num++;
							for (int t = 0; t < 6; t++)
							{
								ng_matrix_sum[t] = (ng_matrix_sum[t] + curr_matrix[t]);
							}
						}
					}
				}
				if (valid_ng_matrix_num == 0)
				{
					cout <<"seed "<<i<< ":2-ring neighbors no SPD matrix, Finally nearest SPD" << endl;
					//cout << "2-ring neighbors no SPD matrix, Finally nearest SPD" << endl;
					/*
					 find the nearest PSD, Y=1/2(X+X^T),eigen decomposition, Y=QDQ^T,D+=max(D,0),X=QD+Q^T,Y=1/2(X+X^T)
					 https://scicomp.stackexchange.com/questions/30631/how-to-find-the-nearest-a-near-positive-definite-from-a-given-matrix
					*/

					vector<float> matrix_m_vec;
					matrix_m_vec.push_back(self_matrix[0]);
					matrix_m_vec.push_back(self_matrix[1]);
					matrix_m_vec.push_back(self_matrix[2]);
					matrix_m_vec.push_back(self_matrix[1]);
					matrix_m_vec.push_back(self_matrix[3]);
					matrix_m_vec.push_back(self_matrix[4]);
					matrix_m_vec.push_back(self_matrix[2]);
					matrix_m_vec.push_back(self_matrix[4]);
					matrix_m_vec.push_back(self_matrix[5]);
					float* m_ptr = &matrix_m_vec[0];
					Map<MatrixXf> m_M(m_ptr, 3, 3);
					Eigen::EigenSolver<MatrixXf> es(m_M);
					VectorXf egvalues = es.eigenvalues().real();
					VectorXf egvalues_plus = egvalues;
					
					for (int a = 0; a < 3; a++)
					{
						//cout << egvalues[a] << "," << egvalues[a] << endl;
						if (egvalues[a] < 0)
						{
							egvalues_plus[a] = 0;
							neg_eigen_value = true;
							//cout << k << ": matrix has non-positive eigenvalues " << endl;// << eigen2[a].real() << endl;
							//break;
						}
						//egvalues_plus[a] += 0.001;
					}

					MatrixXf egvecs = es.eigenvectors().real();
					MatrixXf egvalues_plus_diag;
					MatrixXf nearest_M;
					egvalues_plus_diag = egvalues_plus.asDiagonal();
					nearest_M = egvecs * egvalues_plus_diag*egvecs.transpose();
					//cout << "nearest symmetric positive semidefinite matrix:" << endl << nearest_M << endl;

					seeds_lsc.push_back(nearest_M(0, 0));
					seeds_lsc.push_back(nearest_M(0, 1));
					seeds_lsc.push_back(nearest_M(0, 2));
					seeds_lsc.push_back(nearest_M(1, 1));
					seeds_lsc.push_back(nearest_M(1, 2));
					seeds_lsc.push_back(nearest_M(2, 2));

				}
				else
				{
					//cout << "average of " << valid_ng_matrix_num << " 2-ring neighboring matrix" << endl;
					for (int t = 0; t < 6; t++)
					{
						seeds_lsc.push_back(ng_matrix_sum[t] / valid_ng_matrix_num);
					}
				}
				
			}
			else
			{
				//cout << "average of " << valid_ng_matrix_num << " 1-ring neighboring matrix" << endl;
				for (int t = 0; t < 6; t++)
				{
					seeds_lsc.push_back(ng_matrix_sum[t]/valid_ng_matrix_num);
				}

			}

			
		}
		

		//---------------------------------------------------
	}

	//cout << "!!!non-positive seed matrix num:" << neg_seed_matrix_num << endl;


	glBindVertexArray(VAO_s);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_s);
	glBufferData(GL_ARRAY_BUFFER, seeds_lsc.size() * sizeof(float), &seeds_lsc[0], GL_STATIC_DRAW);//sizeof(pos)只能当pos时数组时才能这样用。比如float pos[]={1,1,1};,这样的话sizeof(pos)=3*sizeof(float)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(4 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(7 * sizeof(float)));
	glEnableVertexAttribArray(3);

}

bool Image::matrix_slover(float seed_x, float seed_y, int on_frame_index, vector<Mat>& flow_forward, vector<Mat>& flow_backward,vector<float>& matrix_6_paras)
{
	//-------------------------------------------------------------
	bool out_error = false;
	bool out_debug = false;
	int ng_x = 0, ng_y = 0;
	vector<float> matrix_A_vec;
	matrix_A_vec.reserve(18 * 6);
	vector<float> vector_b;
	vector_b.reserve(18);
	float expected_b[9] = { 3,2,3,2,1,2,3,2,3 };//one ring neighbor, up to down, left to right

	int id_b = 0;
	for (int m = -1; m < 2; m++)//offset_y
	{
		for (int n = -1; n < 2; n++)//offset_x
		{
			ng_x = int(seed_x) + n;
			ng_y = int(seed_y) + m;
			if (ng_x >= 0 && ng_x < width&&ng_y >= 0 && ng_y < height)
			{
				Vec2f offset(n, m);
				Vec2f delta;
				//forward:one ring neighbor
				if (on_frame_index != (depth - 1))
				{
					delta = flow_forward[on_frame_index].at<Vec2f>(ng_y, ng_x);//(flow_x,flow_y) = at(row,col)
					delta[0] = floor(delta[0] + 0.5);
					delta[1] = floor(delta[1] + 0.5);
					delta = offset + delta;
					if (out_debug)
						cout << delta[0] << "," << delta[1] << ",1" << endl;
					matrix_A_vec.push_back(delta[0] * delta[0]);
					matrix_A_vec.push_back(2 * delta[0] * delta[1]);
					matrix_A_vec.push_back(2 * delta[0]);
					matrix_A_vec.push_back(delta[1] * delta[1]);
					matrix_A_vec.push_back(2 * delta[1]);
					matrix_A_vec.push_back(1);
					vector_b.push_back(expected_b[id_b]);
				}
				//backward:one ring neighbor
				if (on_frame_index != 0)
				{
					delta = flow_backward[on_frame_index].at<Vec2f>(ng_y, ng_x);//(flow_x,flow_y) = at(row,col)
					delta[0] = floor(delta[0] + 0.5);
					delta[1] = floor(delta[1] + 0.5);
					delta = delta + offset;
					if (out_debug)
						cout << delta[0] << "," << delta[1] << ",-1" << endl;
					matrix_A_vec.push_back(delta[0] * delta[0]);
					matrix_A_vec.push_back(2 * delta[0] * delta[1]);
					matrix_A_vec.push_back(-2 * delta[0]);
					matrix_A_vec.push_back(delta[1] * delta[1]);
					matrix_A_vec.push_back(-2 * delta[1]);
					matrix_A_vec.push_back(1);
					vector_b.push_back(expected_b[id_b]);
				}
			}
			id_b++;
		}

	}

	int ng_num = vector_b.size();
	
	float* A_ptr = &matrix_A_vec[0];
	Map<MatrixXf> m_A_temp(A_ptr, 6, ng_num);
	//cout << m_A_temp << endl;
	MatrixXf m_A = m_A_temp.transpose();
	if (out_debug)
	{
		cout << "A:" << endl;
		cout << m_A << endl;
	}
	float* b_ptr = &vector_b[0];
	Map<VectorXf> m_b(b_ptr, ng_num);
	if (out_debug)
	{
		cout << "b:" << endl;
		cout << m_b << endl << endl;
	}
	VectorXf m_m = m_A.bdcSvd(ComputeThinU | ComputeThinV).solve(m_b);
	//matrix_6_paras.clear();
	matrix_6_paras.push_back(m_m[0]);
	matrix_6_paras.push_back(m_m[1]);
	matrix_6_paras.push_back(m_m[2]);
	matrix_6_paras.push_back(m_m[3]);
	matrix_6_paras.push_back(m_m[4]);
	matrix_6_paras.push_back(m_m[5]);


	//std::cout << m_A.colPivHouseholderQr().solve(m_b) << endl;
	if (out_error)
	{
		cout << "sloved m, 6 unknowns:" << endl;
		std::cout << m_m << endl << endl;
	}

	bool neg_eigen_value = false;

	vector<float> matrix_m_vec;
	matrix_m_vec.push_back(m_m[0]);
	matrix_m_vec.push_back(m_m[1]);
	matrix_m_vec.push_back(m_m[2]);
	matrix_m_vec.push_back(m_m[1]);
	matrix_m_vec.push_back(m_m[3]);
	matrix_m_vec.push_back(m_m[4]);
	matrix_m_vec.push_back(m_m[2]);
	matrix_m_vec.push_back(m_m[4]);
	matrix_m_vec.push_back(m_m[5]);
	float* m_ptr = &matrix_m_vec[0];
	Map<MatrixXf> m_M(m_ptr, 3, 3);
	VectorXcf eigen2 = m_M.eigenvalues();
	for (int a = 0; a < 3; a++)
	{
		if (eigen2[a].real() < 0)
		{
			neg_eigen_value = true;
			//cout << i << ": matrix has non-positive eigenvalues " << endl;// << eigen2[a].real() << endl;
			break;
		}
	}

	return neg_eigen_value;
}



void Image::Ld_first_init(Shader& draw_seeds)
{
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_flood);
	draw_seeds.use();
	GLenum draw0_3[4] = { GL_COLOR_ATTACHMENT0 ,GL_COLOR_ATTACHMENT3,GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(4, draw0_3);
	glClearColor(-1.0f, -1.0f, -1.0f, -1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, texture_image[1]);
	draw_seeds.setInt("image", 0);
	draw_seeds.setFloat("width_ratio", width_ratio);
	draw_seeds.setFloat("height_ratio", height_ratio);
	draw_seeds.setFloat("depth_ratio", depth_ratio);
	glBindVertexArray(VAO_s);
	glDrawArrays(GL_POINTS, 0, ac_seeds_num);

	//////已输出验证结果正确
	//glBindTexture(GL_TEXTURE_3D, texture_flood[0]);
	//print_buffer(0);
	//glBindTexture(GL_TEXTURE_3D, texture_flood[3]);
	//print_buffer(3);
	//glBindTexture(GL_TEXTURE_3D, texture_flood[2]);
	//print_buffer(2);
	//glBindTexture(GL_TEXTURE_3D, texture_flood[4]);
	//print_buffer(4);

}

void Image::init_variables_attributes(int& ac_seeds_num_)
{
	compactness = 6;
	ratio = (compactness / s)*(compactness / s);//0.01;
	cout << "compactness= " << compactness << ",ratio=" << ratio << endl;

	ac_seeds_num = ac_seeds_num_;


	vector<float> seeds_index;//tex_x,tex_y,tex_z,id
	seeds_index.reserve(ac_seeds_num * 4);
	int s_index = 0;
	float tex_x = 0, tex_y = 0, tex_z = 0;
	for (int i = 0; i < ac_seeds_num; i++)
	{
		tex_x = float(s_index % width) + 0.5;
		tex_y = float(s_index / width) + 0.5;
		tex_z = float(s_index / slice_num) + 0.5;

		tex_x = tex_x * width_ratio;
		tex_y = tex_y * height_ratio;
		tex_z = tex_z * depth_ratio;
		seeds_index.push_back(tex_x);
		seeds_index.push_back(tex_y);
		seeds_index.push_back(tex_z);
		seeds_index.push_back(float(i));
		s_index++;
	}
	glGenVertexArrays(1, &VAO_s_index);
	glGenBuffers(1, &VBO_s_index);
	glBindVertexArray(VAO_s_index);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_s_index);
	glBufferData(GL_ARRAY_BUFFER, seeds_index.size() * sizeof(float), &seeds_index[0], GL_STATIC_DRAW);//sizeof(pos)只能当pos时数组时才能这样用。比如float pos[]={1,1,1};,这样的话sizeof(pos)=3*sizeof(float)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);




	vector<short> all_texels_int;
	all_texels_int.reserve(texel_num * 3);
	for (int i = 0; i < depth; i++)
	{
		for (int j = 0; j < height; j++)
		{
			for (int t = 0; t < width; t++)
			{
				all_texels_int.push_back(short(t));
				all_texels_int.push_back(short(j));
				all_texels_int.push_back(short(i));
			}
		}
	}

	glGenVertexArrays(1, &VAO_all_t);
	glGenBuffers(1, &VBO_all_t);
	glBindVertexArray(VAO_all_t);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_all_t);
	glBufferData(GL_ARRAY_BUFFER, all_texels_int.size() * sizeof(short), &all_texels_int[0], GL_STATIC_DRAW);
	glVertexAttribIPointer(0, 3, GL_SHORT, 3 * sizeof(short), (void*)0);
	glEnableVertexAttribArray(0);

}

void Image::reset_variables()
{
	//step_length = (max_s + 1)*0.5*0.5;
	step_length = floor(0.5*s);
	/*current_buffer = 1;*/
	flood_iter = 0;
	seed_range = 0;
}
void Image::flood(Shader& flood, int Lloyd_curr_iter)
{
	
	reset_variables();

	flood.use();
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_flood);

	float lambda = 1;
	float last2 = 2;


	//执行1+JFA
	seed_range += 1;
	//printf("%d iter:1\n", flood_iter);
	glDrawBuffers(1, buffers_flood + current_buffer);
	glClearColor(-1.0f, -1.0f, -1.0f, -1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, texture_flood[1 - current_buffer]);
	flood.setInt("texels", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, texture_flood[3]);
	flood.setInt("sColorImage", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, texture_flood[2]);
	flood.setInt("seedMatrixFirst3", 2);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_3D, texture_flood[4]);
	flood.setInt("seedMatrixLast3", 3);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_3D, texture_image[1]);
	flood.setInt("image", 4);
	flood.setFloat("width_ratio", width_ratio);
	flood.setFloat("height_ratio", height_ratio);
	flood.setFloat("depth_ratio", depth_ratio);
	flood.setFloat("step", 1);
	flood.setFloat("ratio", ratio);
	glBindVertexArray(VAO_all_t);
	glDrawArrays(GL_POINTS, 0, texel_num);
	/*glBindTexture(GL_TEXTURE_3D, texture_flood[current_buffer]);
	print_buffer(current_buffer);*/
	current_buffer = 1 - current_buffer;
	flood_iter++;

	while (step_length > 0)
	{
		seed_range += step_length;
		//printf("%d iter:%f\n", flood_iter, step_length);
		glDrawBuffers(1, buffers_flood + current_buffer);
		glClearColor(-1.0f, -1.0f, -1.0f, -1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, texture_flood[1 - current_buffer]);
		flood.setInt("texels", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_3D, texture_flood[3]);
		flood.setInt("sColorImage", 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_3D, texture_flood[2]);
		flood.setInt("seedMatrixFirst3", 2);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_3D, texture_flood[4]);
		flood.setInt("seedMatrixLast3", 3);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_3D, texture_image[1]);
		flood.setInt("image", 4);
		flood.setFloat("width_ratio", width_ratio);
		flood.setFloat("height_ratio", height_ratio);
		flood.setFloat("depth_ratio", depth_ratio);
		flood.setFloat("step", step_length);
		flood.setFloat("ratio", lambda*ratio);
		glBindVertexArray(VAO_all_t);
		glDrawArrays(GL_POINTS, 0, texel_num);

		flood_iter++;
		if (int(step_length) == 1)
			step_length = 0;
		step_length = floor(0.5*step_length + 0.5);
		current_buffer = 1 - current_buffer;
	}

	
	{
		step_length = 2;
		while (step_length > 0)
		{
			seed_range += step_length;
			//printf("%d iter:%f\n", flood_iter, step_length);
			glDrawBuffers(1, buffers_flood + current_buffer);
			glClearColor(-1.0f, -1.0f, -1.0f, -1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_3D, texture_flood[1 - current_buffer]);
			flood.setInt("texels", 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_3D, texture_flood[3]);
			flood.setInt("sColorImage", 1);
			
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_3D, texture_flood[2]);
			flood.setInt("seedMatrixFirst3", 2);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_3D, texture_flood[4]);
			flood.setInt("seedMatrixLast3", 3);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_3D, texture_image[1]);
			flood.setInt("image", 4);
			flood.setFloat("width_ratio", width_ratio);
			flood.setFloat("height_ratio", height_ratio);
			flood.setFloat("depth_ratio", depth_ratio);
			flood.setFloat("step", step_length);
			flood.setFloat("ratio", last2 * ratio);
			glBindVertexArray(VAO_all_t);
			glDrawArrays(GL_POINTS, 0, texel_num);
			if (int(step_length) == 1)
				step_length = 0;
			step_length = floor(0.5*step_length + 0.5);
			current_buffer = 1 - current_buffer;
			flood_iter++;
		}
	}




	/*if (Lloyd_curr_iter == Lloyd_max_iter)
	{
		glBindTexture(GL_TEXTURE_3D, texture_flood[1 - current_buffer]);
		print_final(1 - current_buffer);
	}*/


}



void Image::print_seg(unsigned char* data)
{
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_flood);
	glBindTexture(GL_TEXTURE_3D, texture_flood[1 - current_buffer]);
	float* res_flood = new float[texel_num * 4];//GL_RGB
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, res_flood);

	int* res_flood_one = new int[width*height*depth];
	int index = 3;
	for (int i = 0; i < texel_num; i++)
	{
		res_flood_one[i] = int(res_flood[index]);
		index += 4;
	}
	index = 0;
	int index_r = 0, index_d = 0;
	int index3 = 0;
	for (int k = 0; k < depth; k++)
	{
		for (int i = 0; i < height - 1; i++)
		{
			for (int j = 0; j < width - 1; j++)
			{
				index = k * slice_num + i * width + j;
				index_r = index + 1;
				index_d = index + width;

				if ((res_flood_one[index] != res_flood_one[index_r]) || (res_flood_one[index] != res_flood_one[index_d]))
				{
					data[3 * index] = 255;
					data[3 * index + 1] = 0;
					data[3 * index + 2] = 0;
				}
			}
		}
	}

	unsigned char* slice_start = NULL;
	namedWindow("Display window", cv::WINDOW_AUTOSIZE);
	for (int i = 0; i < depth; i++)
	{
		if (i == 0)
			slice_start = data;
		else
			slice_start = slice_start + 3 * slice_num;
		Mat seg(Size(width, height), CV_8UC3, slice_start);
		cvtColor(seg, seg, COLOR_BGR2RGB);
		flip(seg, seg, 0);//rows-i-1,j
		imshow("Display window", seg);
		waitKey();

	}

}


void Image::centroids_next_seeds(Shader& centroids_next_seeds, Shader& aver_exception, int Lloyd_curr_iter)
{
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_flood);

	centroids_next_seeds.use();
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);

	GLenum drawcurr_2[2] = { buffers_flood[current_buffer],buffers_flood[3] };
	glDrawBuffers(2, drawcurr_2);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, texture_flood[1 - current_buffer]);
	centroids_next_seeds.setInt("texels", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, texture_image[1]);
	centroids_next_seeds.setInt("image", 1);
	centroids_next_seeds.setFloat("width_ratio", width_ratio);
	centroids_next_seeds.setFloat("height_ratio", height_ratio);
	centroids_next_seeds.setFloat("depth_ratio", depth_ratio);
	centroids_next_seeds.setFloat("ac_seeds_num", float(ac_seeds_num));
	glBindVertexArray(VAO_all_t);
	glDrawArrays(GL_POINTS, 0, texel_num);

	/*glBindTexture(GL_TEXTURE_3D, texture_flood[current_buffer]);
	print_buffer(current_buffer);*/
	/*glBindTexture(GL_TEXTURE_3D, texture_flood[3]);
	print_buffer(3);*/

	
	glDisable(GL_BLEND);

	
	if (Lloyd_curr_iter !=Lloyd_max_iter)
	{
		current_buffer = 1 - current_buffer;

		drawcurr_2[0] = buffers_flood[current_buffer];
		drawcurr_2[1] = buffers_flood[2];
		aver_exception.use();
		glDrawBuffers(2, drawcurr_2);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, texture_flood[1 - current_buffer]);
		aver_exception.setInt("sum_pos", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_3D, texture_flood[3]);
		aver_exception.setInt("sum_color", 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_3D, texture_image[1]);
		aver_exception.setInt("image", 2);
		aver_exception.setFloat("width", width_f);
		aver_exception.setFloat("height", height_f);
		aver_exception.setFloat("depth", depth_f);
		glBindVertexArray(VAO_s_index);
		glDrawArrays(GL_POINTS, 0, ac_seeds_num);
		/*glBindTexture(GL_TEXTURE_3D, texture_flood[current_buffer]);
		print_buffer(current_buffer);*/

		current_buffer = 1 - current_buffer;

		
	}
}



void Image::Ld_next_seeds_matrix(Shader& draw_next_seeds, vector<Mat>& flow_forward, vector<Mat>& flow_backward, int Lloyd_curr_iter)
{
	
	float error_thd = 1;
	int error_range1 = 0, error_range2 = 0, error_range3 = 0, error_range4 = 0;
	float flow_x = 0, flow_y = 0;
	bool out_debug = false;
	bool out_error = false;

	cout << "Lloyd iteration: " << Lloyd_curr_iter << endl;

	int read_height = (ac_seeds_num / width) + 1;
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_flood);
	glBindTexture(GL_TEXTURE_3D, texture_flood[1 - current_buffer]);
	float* res_pos = new float[width*read_height * 4];
	glReadBuffer(buffers_flood[1 - current_buffer]);
	glReadPixels(0, 0, width, read_height, GL_RGBA, GL_FLOAT, res_pos);


	glBindTexture(GL_TEXTURE_3D, texture_flood[2]);
	float* res_color = new float[width*read_height * 4];
	glReadBuffer(buffers_flood[2]);
	glReadPixels(0, 0, width, read_height, GL_RGBA, GL_FLOAT, res_color);

	seeds_lsc.clear();
	seeds_lsc.reserve(ac_seeds_num * 13);//x,y,z,id,l,a,b,symmetric matrix 6 unknowns
	
	int on_frame_index = 0;
	float seed_x = 0, seed_y = 0, seed_z = 0;
	int ng_x = 0, ng_y = 0;
	int index = 0;
	vector<float> curr_matrix;
	vector<float> self_matrix;
	int neg_seed_matrix_num = 0;
	int ng_1ring_x[8] = { -1,0,1,-1,1,-1,0,1 };
	int ng_1ring_y[8] = { -1,-1,-1,0,0,1,1,1 };

	for (int i = 0; i < ac_seeds_num; i++)
	{
		seed_x = res_pos[index];
		seed_y = res_pos[index + 1];
		seed_z = res_pos[index + 2];
		on_frame_index = int(seed_z);
		assert(i == int(res_pos[index + 3]));
		seeds_lsc.push_back(seed_x);
		seeds_lsc.push_back(seed_y);
		seeds_lsc.push_back(seed_z);
		seeds_lsc.push_back(float(i));
		seeds_lsc.push_back(res_color[index]);
		seeds_lsc.push_back(res_color[index + 1]);
		seeds_lsc.push_back(res_color[index + 2]);
		index += 4;
		seed_x = floor(seed_x);
		seed_y = floor(seed_y);
		
		curr_matrix.clear();
		bool neg_eigen_value = matrix_slover(seed_x, seed_y, on_frame_index, flow_forward, flow_backward, curr_matrix);
		self_matrix.clear();
		self_matrix.reserve(6);
		self_matrix.assign(curr_matrix.begin(), curr_matrix.end());

		if (!neg_eigen_value)
		{
			for (int t = 0; t < 6; t++)
			{
				seeds_lsc.push_back(curr_matrix[t]);
			}
		}
		else
		{
			neg_seed_matrix_num++;
			//cout << "seed " << i << ": ";
			int valid_ng_matrix_num = 0;
			vector<float> ng_matrix_sum;
			for (int t = 0; t < 6; t++)
			{
				ng_matrix_sum.push_back(0);
			}
			float seed_ng_x = 0, seed_ng_y = 0;
			for (int t = 0; t < 8; t++)
			{
				curr_matrix.clear();
				seed_ng_x = seed_x + ng_1ring_x[t];
				seed_ng_y = seed_y + ng_1ring_y[t];
				if (seed_ng_x >= 0 && seed_ng_x < width&&seed_ng_y >= 0 && seed_ng_y < height)
				{
					neg_eigen_value = matrix_slover(seed_ng_x, seed_ng_y, on_frame_index, flow_forward, flow_backward, curr_matrix);
					if (!neg_eigen_value)
					{
						valid_ng_matrix_num++;
						for (int t = 0; t < 6; t++)
						{
							ng_matrix_sum[t] = (ng_matrix_sum[t] + curr_matrix[t]);
						}
					}
				}
			}
			if (valid_ng_matrix_num == 0)
			{
				int ng_2ring_x[16] = { -2,-1,0,1,2,-2,2,-2,2,-2,2,-2,-1,0,1,2 };
				int ng_2ring_y[16] = { -2,-2,-2,-2,-2,-1,-1,0,0,1,1,2,2,2,2,2 };
				for (int t = 0; t < 16; t++)
				{
					curr_matrix.clear();
					seed_ng_x = seed_x + ng_2ring_x[t];
					seed_ng_y = seed_y + ng_2ring_y[t];
					if (seed_ng_x >= 0 && seed_ng_x < width&&seed_ng_y >= 0 && seed_ng_y < height)
					{
						neg_eigen_value = matrix_slover(seed_ng_x, seed_ng_y, on_frame_index, flow_forward, flow_backward, curr_matrix);
						if (!neg_eigen_value)
						{
							valid_ng_matrix_num++;
							for (int t = 0; t < 6; t++)
							{
								ng_matrix_sum[t] = (ng_matrix_sum[t] + curr_matrix[t]);
							}
						}
					}
				}
				if (valid_ng_matrix_num == 0)
				{
					cout <<"seed "<<i << ": 2-ring neighbors no SPD matrix, Finally nearest SPD" << endl;
					vector<float> matrix_m_vec;
					matrix_m_vec.push_back(self_matrix[0]);
					matrix_m_vec.push_back(self_matrix[1]);
					matrix_m_vec.push_back(self_matrix[2]);
					matrix_m_vec.push_back(self_matrix[1]);
					matrix_m_vec.push_back(self_matrix[3]);
					matrix_m_vec.push_back(self_matrix[4]);
					matrix_m_vec.push_back(self_matrix[2]);
					matrix_m_vec.push_back(self_matrix[4]);
					matrix_m_vec.push_back(self_matrix[5]);
					float* m_ptr = &matrix_m_vec[0];
					Map<MatrixXf> m_M(m_ptr, 3, 3);
					Eigen::EigenSolver<MatrixXf> es(m_M);
					VectorXf egvalues = es.eigenvalues().real();
					VectorXf egvalues_plus = egvalues;
					for (int a = 0; a < 3; a++)
					{
						//cout << egvalues[a] << "," << egvalues[a] << endl;
						if (egvalues[a] < 0)
						{
							egvalues_plus[a] = 0;
							neg_eigen_value = true;
							//cout << k << ": matrix has non-positive eigenvalues " << endl;// << eigen2[a].real() << endl;
							//break;
						}
						//egvalues_plus[a] += 0.001;
					}

					MatrixXf egvecs = es.eigenvectors().real();
					MatrixXf egvalues_plus_diag;
					MatrixXf nearest_M;
					egvalues_plus_diag = egvalues_plus.asDiagonal();
					nearest_M = egvecs * egvalues_plus_diag*egvecs.transpose();
					//cout << "nearest symmetric positive semidefinite matrix:" << endl << nearest_M << endl;

					seeds_lsc.push_back(nearest_M(0, 0));
					seeds_lsc.push_back(nearest_M(0, 1));
					seeds_lsc.push_back(nearest_M(0, 2));
					seeds_lsc.push_back(nearest_M(1, 1));
					seeds_lsc.push_back(nearest_M(1, 2));
					seeds_lsc.push_back(nearest_M(2, 2));

				}
				else
				{
					//cout << "average of " << valid_ng_matrix_num << " 2-ring neighboring matrix" << endl;
					for (int t = 0; t < 6; t++)
					{
						seeds_lsc.push_back(ng_matrix_sum[t] / valid_ng_matrix_num);
					}
				}
			}
			else
			{
				//cout << "average of " << valid_ng_matrix_num << " 1-ring neighboring matrix" << endl;
				for (int t = 0; t < 6; t++)
				{
					seeds_lsc.push_back(ng_matrix_sum[t] / valid_ng_matrix_num);
				}
			}

		}

	}
	//cout << "!!!non-positive seed matrix num:" << neg_seed_matrix_num << endl;


	if (res_pos) delete[] res_pos;
	if (res_color) delete[] res_color;


	glBindVertexArray(VAO_s);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_s);
	glBufferData(GL_ARRAY_BUFFER, seeds_lsc.size() * sizeof(float), &seeds_lsc[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)(4 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)(7 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)(10 * sizeof(float)));
	glEnableVertexAttribArray(4);

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_flood);
	draw_next_seeds.use();
	GLenum draw0_3[4] = { buffers_flood[current_buffer] ,GL_COLOR_ATTACHMENT3,GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(4, draw0_3);
	glClearColor(-1.0f, -1.0f, -1.0f, -1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	draw_next_seeds.setFloat("width_ratio", width_ratio);
	draw_next_seeds.setFloat("height_ratio", height_ratio);
	draw_next_seeds.setFloat("depth_ratio", depth_ratio);
	glBindVertexArray(VAO_s);
	glDrawArrays(GL_POINTS, 0, ac_seeds_num);

	/*glBindTexture(GL_TEXTURE_3D, texture_flood[current_buffer]);
	print_buffer(current_buffer);
	glBindTexture(GL_TEXTURE_3D, texture_flood[3]);
	print_buffer(3);*/

	current_buffer = 1 - current_buffer;

}


void Image::check_dead_seeds(Shader& label, Shader& label_replace, int* labels,vector<float>& seeds_info)
{
	time_t start = 0, end = 0;
	double time_ms = 0;


	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_flood);
	glBindTexture(GL_TEXTURE_3D,texture_flood[current_buffer]);
	int read_height = ((ac_seeds_num+1) / width)+1;
	float* res1 = new float[width*read_height * 4];
	glReadBuffer(buffers_flood[current_buffer]);
	glReadPixels(0, 0, width, read_height, GL_RGBA, GL_FLOAT, res1);
	
	bool invalid_label_exist = false;
	if (res1[ac_seeds_num * 4 + 3] > 0)
	{
		invalid_label_exist = true;
		cout << "!!!-1 label exist,num："<< res1[ac_seeds_num * 4 + 3] << endl;
	}
	
	
	vector<int> seed_invalid;
	int dead_seed_num = 0, dead_seed_index = -1, last_seed_index = ac_seeds_num;
	int valid_seed_num = 0;
	seed_invalid.reserve(2);
	int curr_seed_area = 0;
	map<int, int> sequence_label_map;
	int index = 0;
	seeds_info.clear();
	for (int i = 0; i < ac_seeds_num; i++)
	{
		curr_seed_area = int(res1[index+3]);
		if (curr_seed_area < 1)
		{
			dead_seed_num++;
			seed_invalid.push_back(i);
		}
		else
		{
			seeds_info.push_back(floor(res1[index] / curr_seed_area) + 0.5);
			seeds_info.push_back(floor(res1[index+1] / curr_seed_area) + 0.5);
			seeds_info.push_back(floor(res1[index+2] / curr_seed_area) + 0.5);
			seeds_info.push_back(i);
			sequence_label_map.insert(pair<int, int>(i, valid_seed_num));
			valid_seed_num++;
		}

		index += 4;
	}
	if (res1) delete[] res1;
	cout << "!!!dead seeds num:"<<dead_seed_num << endl;
	final_label_num = valid_seed_num;
	cout << "final label num:" << final_label_num << endl;


	if (dead_seed_num == 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_image);
		label.use();
		glDrawBuffers(1, buffers_image + 3);
		glClearColor(3.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, texture_flood[1 - current_buffer]);
		label.setInt("texels", 0);
		label.setFloat("width_ratio", width_ratio);
		label.setFloat("height_ratio", height_ratio);
		label.setFloat("depth_ratio", depth_ratio);
		glBindVertexArray(VAO_all_t);
		glDrawArrays(GL_POINTS, 0, texel_num);

		glBindTexture(GL_TEXTURE_3D, texture_image[3]);
		glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_INT, labels);

		if (invalid_label_exist)
			sequence_label(labels, sequence_label_map, false);
	}
	else if (dead_seed_num<=3)
	{
		for (int i = 0; i < seed_invalid.size(); i++)
		{
			dead_seed_index = seed_invalid[i];
			cout << "seed " << dead_seed_index << " dead!" << endl;
			last_seed_index -= 1;

			label_replace.use();
			glDrawBuffers(1, buffers_flood + current_buffer);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_3D, texture_flood[1 - current_buffer]);
			label_replace.setInt("texels", 0);
			label_replace.setFloat("width_ratio", width_ratio);
			label_replace.setFloat("height_ratio", height_ratio);
			label_replace.setFloat("depth_ratio", depth_ratio);
			label_replace.setInt("dead_seed_index", dead_seed_index);
			label_replace.setInt("last_seed_index", last_seed_index);
			glBindVertexArray(VAO_all_t);
			glDrawArrays(GL_POINTS, 0, texel_num);

			current_buffer = 1 - current_buffer;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_image);
		label.use();
		glDrawBuffers(1, buffers_image + 3);
		glClearColor(3.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, texture_flood[1 - current_buffer]);
		label.setInt("texels", 0);
		label.setFloat("width_ratio", width_ratio);
		label.setFloat("height_ratio", height_ratio);
		label.setFloat("depth_ratio", depth_ratio);
		glBindVertexArray(VAO_all_t);
		glDrawArrays(GL_POINTS, 0, texel_num);

		glBindTexture(GL_TEXTURE_3D, texture_image[3]);
		glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_INT, labels);
		
		if (invalid_label_exist)
			sequence_label(labels, sequence_label_map, false);
	}
	else if (dead_seed_num > 3)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_image);
		label.use();
		glDrawBuffers(1, buffers_image + 3);
		glClearColor(3.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, texture_flood[1 - current_buffer]);
		label.setInt("texels", 0);
		label.setFloat("width_ratio", width_ratio);
		label.setFloat("height_ratio", height_ratio);
		label.setFloat("depth_ratio", depth_ratio);
		glBindVertexArray(VAO_all_t);
		glDrawArrays(GL_POINTS, 0, texel_num);

		glBindTexture(GL_TEXTURE_3D, texture_image[3]);
		glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_INT, labels);
		cout << "need call sequence_label()！" << endl;
		sequence_label(labels, sequence_label_map,true);
	}



}

void Image::sequence_label(int* labels, map<int, int>& sequence_label_map, bool dead_seed)
{
	if (dead_seed)
	{
		cout << "deal with label discontinuity" << endl;
		int last_label = 0;
		int last_label_map = 0;
		last_label = labels[0];
		last_label_map = sequence_label_map[last_label];
		labels[0] = last_label_map;
		for (int i = 1; i < texel_num; i++)
		{
			if (labels[i] != last_label)
			{
				if (labels[i] != -1)
					last_label_map = sequence_label_map[labels[i]];
			}
			labels[i] = last_label_map;
		}
	}
	else
	{
		cout << "deal with -1 label" << endl;
		int invalid_num = 0;
		int last_label = 0;
		if (labels[0] < 0)
		{
			invalid_num++;
		}
		else
		{
			last_label = labels[0];
		}
		//last_label = labels[0];
		for (int i = 1; i < texel_num; i++)
		{
			if (labels[i] != last_label)
			{
				if (labels[i] == -1)
				{
					labels[i] = last_label;
					invalid_num++;
				}
					
				last_label = labels[i];
			}
		}
		cout << "label -1 num: " << invalid_num << endl;
	}
	
	
}

void Image::label_notconnected(int* labels, string dirName, bool output_label)
{
	if (output_label)
	{
		string txtname = dirName + ".txt";
		ofstream f_debug1(txtname);
		int slice_index = 0;
		for (int t = 0; t < depth; t++)
		{
			for (int i = height - 1; i >= 0; i--) {
				slice_index = t * slice_num + i * width;
				for (int j = 0; j < width; j++) {
					f_debug1 << std::left << std::setw(4) << int(labels[slice_index]) << " ";//注意这里之前输出res了，处理离散最后应该输出labels!!!
					slice_index++;
				}
				f_debug1 << endl;
			}
			f_debug1 << endl;
			f_debug1 << endl;
		}
		f_debug1.close();
		cout << "save not connected label!!!" << endl;
	}
}

void Image::enforce_connectivity_3d(Shader& label,int* labels,int& label_num,string dirName,bool output_label)
{
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_image);
	label.use();
	glDrawBuffers(1, buffers_image + 3);
	glClearColor(3.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, texture_flood[1 - current_buffer]);
	label.setInt("texels", 0);
	label.setFloat("width_ratio", width_ratio);
	label.setFloat("height_ratio", height_ratio);
	label.setFloat("depth_ratio", depth_ratio);
	glBindVertexArray(VAO_all_t);
	glDrawArrays(GL_POINTS, 0, texel_num);

	/*glBindTexture(GL_TEXTURE_3D, texture_image[3]);
	print_image_label();*/

	glBindTexture(GL_TEXTURE_3D, texture_image[3]);
	int* res = new int[texel_num];
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_INT, res);

	
	
	final_label_num = ac_seeds_num;
	int SUPSZ = texel_num / seeds_num;
	EnforceLabelConnectivity3D(res,labels, final_label_num, SUPSZ);
	label_num = final_label_num;


	
	if (output_label)
	{
		string txtname = dirName + ".txt";
		ofstream f_debug1(txtname);
		int slice_index = 0;
		for (int t = 0; t < depth; t++)
		{
			for (int i = height - 1; i >= 0; i--) {
				slice_index = t * slice_num + i * width;
				for (int j = 0; j < width; j++) {
					f_debug1 << std::left << std::setw(4) << int(labels[slice_index]) << " ";//注意这里之前输出res了，处理离散最后应该输出labels!!!
					slice_index++;
				}
				f_debug1 << endl;
			}
			f_debug1 << endl;
			f_debug1 << endl;
		}
		f_debug1.close();
		cout << "save label!!!" << endl;
	}
	

	if (res) delete[] res;
}

void Image::print_connect_seg(unsigned char* data, int* labels)
{
	int index = 0;
	int index_r = 0, index_d = 0;
	int index3 = 0;
	for (int k = 0; k < depth; k++)
	{
		for (int i = 0; i < height - 1; i++)
		{
			for (int j = 0; j < width - 1; j++)
			{
				index = k * slice_num + i * width + j;
				index_r = index + 1;
				index_d = index + width;

				if ((labels[index] != labels[index_r]) || (labels[index] != labels[index_d]))
				{
					data[3 * index] = 255;
					data[3 * index + 1] = 0;
					data[3 * index + 2] = 0;
				}
			}
		}
	}

	unsigned char* slice_start = NULL;
	//namedWindow("Display window", cv::WINDOW_AUTOSIZE);
	for (int i = 0; i < depth; i++)
	{
		if (i == 0)
			slice_start = data;
		else
			slice_start = slice_start + 3 * slice_num;
		Mat seg(Size(width, height), CV_8UC3, slice_start);
		cvtColor(seg, seg, COLOR_BGR2RGB);
		flip(seg, seg, 0);//rows-i-1,j
		imshow("Image Segs", seg);
		waitKey();
		/*string name = "voxel\\ice" + to_string(i) + ".bmp";
		imwrite(name, seg);*/
	}
}

void Image::print_render(int* labels,int start_index)
{
	Mat render(Size(width, height*depth), CV_8UC3, Scalar(0, 0, 0));
	unsigned char* data = render.data;
	int* render_color = new int[final_label_num * 3];
	for (int i = 0; i < final_label_num * 3; i++)
	{
		render_color[i] = rand() % 256;
	}

	int label_index = 0;
	int label_va = 0;
	uchar* ptr = NULL;
	for (int i = 0; i < (depth*height); i++)
	{
		ptr = render.ptr<uchar>(i);
		for (int j = 0; j < width; j++)
		{
			label_va = 3 * labels[label_index++];
			if (label_va< 0) 
			{
				*(ptr++) = 0;
				*(ptr++) = 0;
				*(ptr++) = 0;
			}
			else
			{
				*(ptr++) = render_color[label_va];
				*(ptr++) = render_color[label_va + 1];
				*(ptr++) = render_color[label_va + 2];
			}
			
		}
	}



	unsigned char* slice_start = NULL;
	for (int i = 0; i < depth; i++)
	{
		if (i == 0)
			slice_start = render.data;
		else
			slice_start = slice_start + 3 * slice_num;
		
		Mat seg(Size(width, height), CV_8UC3, slice_start);
		cvtColor(seg, seg, COLOR_BGR2RGB);
		flip(seg, seg, 0);//rows-i-1,j
		/*imshow("JFAInitMatrix", seg);
		waitKey();*/

		char savepath[1024]; 
		sprintf_s(savepath, "voxel\\test\\%05d_%05d.png", start_index,(i +start_index+ 1));
		imwrite(savepath, seg);
	}


       	if (render_color) delete[] render_color;
}


void Image::EnforceLabelConnectivity3D(
	int*					labels,//input labels that need to be corrected to remove stray labels
	int*&						nlabels,//new labels
	int&						numlabels,//the number of labels changes in the end if segments are removed
	const int&					SUPSZ) //the average size of superpixel, based on seeds' num desired by user
{
	const int dx8[8] = {-1, -1,  0,  1, 1, 1, 0, -1};
	const int dy8[8] = { 0, -1, -1, -1, 0, 1, 1,  1};

	/*const int dx4[4] = { -1, 0, 1, 0 };
	const int dy4[4] = { 0, -1, 0, 1 };*/

	const int dz3[3] = { -1,0,1 };


	//nlabels.resize(sz, -1);
	for (int i = 0; i < texel_num; i++)
	{
		nlabels[i] = -1;
	}
	int label(0);//index of superpixel starts from 0！
	//seed_range = 10*pow(seed_range, 3);
	int* xvec = new int[texel_num];
	int* yvec = new int[texel_num];
	int* zvec = new int[texel_num];
	int oindex(0);
	int adjlabel(0);//adjacent label
	int x = 0, y = 0, z = 0;
	bool merge_flag = false;
	for (int i = 0; i < depth; i++)
	{
		for (int j = 0; j < height; j++)
		{
			for (int k = 0; k < width; k++)
			{
				if (0 > nlabels[oindex])
				{
					nlabels[oindex] = label;
					//--------------------
					// Start a new segment
					//--------------------
					xvec[0] = k;
					yvec[0] = j;
					zvec[0] = i;
					//---- ---------------------------------------------------
					// Quickly find an adjacent label for use later if needed
					//-------------------------------------------------------
					for (int m = 0; m < 3; m++)
					{
						z = zvec[0] + dz3[m];
						if (z >= 0 && z < depth)
						{
							for (int n = 0; n < 8; n++)
							{
								x = xvec[0] + dx8[n];
								y = yvec[0] + dy8[n];
								if ((x >= 0 && x < width) && (y >= 0 && y < height))
								{
									int nindex = z * slice_num + y * width + x;
									if (nlabels[nindex] >= 0) adjlabel = nlabels[nindex];
								}
							}
						}

					}


					int frame_count = 0;
					int count(1);
					for (int c = 0; c < count; c++)
					{
						for (int m = 0; m < 3; m++)
						{
							z = zvec[c] + dz3[m];

							if (z >= 0 && z < depth)
							{
								for (int n = 0; n < 8; n++)
								{
									x = xvec[c] + dx8[n];
									y = yvec[c] + dy8[n];

									if ((x >= 0 && x < width) && (y >= 0 && y < height))
									{
										int nindex = z * slice_num + y * width + x;

										if (0 > nlabels[nindex] && (labels[oindex] == labels[nindex]))
										{
											xvec[count] = x;
											yvec[count] = y;
											zvec[count] = z;
											nlabels[nindex] = label;
											count++;

										}

									}

								}

							}

						}

					}

					//-------------------------------------------------------
					// If segment size is less then a limit, assign an
					// adjacent label found before, and decrement label count.
					//-------------------------------------------------------
					if (count < (SUPSZ /4))
					{
						for (int c = 0; c < count; c++)
						{
							int ind = zvec[c] * slice_num + yvec[c] * width + xvec[c];
							nlabels[ind] = adjlabel;
						}
						label--;
					}
					

					
						
					label++;
				}
				oindex++;
			}
		}
	}

	numlabels = label;

	if (xvec) delete[] xvec; //xvec = NULL;
	if (yvec) delete[] yvec;// yvec = NULL;
	if (zvec) delete[] zvec;// zvec = NULL;
}





void Image::print_image_label()
{
	int* res = new int[texel_num];
	//float* res = new float[texel_num];
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_INT, res);
	string txtname = "image_label.txt";
	ofstream f_debug1(txtname);
	int slice_index = 0;
	for (int t = 0; t < depth; t++)
	{
		//slice_index = t * width*height * 3;
		for (int i = height - 1; i >= 0; i--) {
			slice_index = t * slice_num + i * width;
			for (int j = 0; j < width; j++) {
				f_debug1 << std::left << std::setw(4) << int(res[slice_index]) << " ";
				slice_index++;
			}
			f_debug1 << endl;
		}
		f_debug1 << endl;
		f_debug1 << endl;
	}

	f_debug1.close();

	if (res) delete[] res;
}

void Image::print_image_label_float()
{
	float* res = new float[texel_num];
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RED, GL_FLOAT, res);
	string txtname = "image_label_float.txt";
	ofstream f_debug1(txtname);
	int slice_index = 0;
	for (int t = 0; t < depth; t++)
	{
		//slice_index = t * width*height * 3;
		for (int i = height - 1; i >= 0; i--) {
			slice_index = t * slice_num + i * width;
			for (int j = 0; j < width; j++) {
				f_debug1 << std::left << std::setw(4) << int(res[slice_index]) << " ";
				slice_index++;
			}
			f_debug1 << endl;
		}
		f_debug1 << endl;
		f_debug1 << endl;
	}

	f_debug1.close();

	if (res) delete[] res;
}


void Image::print_final(int index)
{
	float* res1 = new float[texel_num * 4];//GL_RGBA
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, res1);
	string txtname = "rgb_flood" + std::to_string(index) + ".txt";
	ofstream f_debug1(txtname);
	int slice_index = 3;
	for (int t = 0; t < depth; t++)
	{
		//slice_index = t * width*height * 3;
		for (int i = height - 1; i >= 0; i--) {
			slice_index = t * slice_num + i * width + 3;
			for (int j = 0; j < width; j++) {
				f_debug1 << std::left << std::setw(4) << res1[slice_index] << " ";
				slice_index += 4;
			}
			f_debug1 << endl;
		}
		f_debug1 << endl;
		f_debug1 << endl;
	}

	f_debug1.close();
	if (res1) delete[] res1;
}


void Image::print_buffer_1float(int index)
{
	float* res1 = new float[texel_num];//GL_RGBA
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RED, GL_FLOAT, res1);
	string txtname = "gradient_onechannel" + std::to_string(index) + ".txt";
	ofstream f_debug1(txtname);
	int slice_index = 0;
	for (int t = 0; t < depth; t++)
	{
		//slice_index = t * width*height * 3;
		for (int i = height - 1; i >= 0; i--) {
			slice_index = (t * slice_num + i * width);
			for (int j = 0; j < width; j++) {
				f_debug1 << std::left << std::setw(4) << res1[slice_index++] << " ";

			}
			f_debug1 << endl;
		}
		f_debug1 << endl;
		f_debug1 << endl;
	}

	f_debug1.close();
	if (res1) delete[] res1;
}


void Image::print_buffer(int index)
{
	float* res1 = new float[texel_num * 4];//GL_RGBA
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, res1);
	string txtname = "rgb" + std::to_string(index) + ".txt";
	ofstream f_debug1(txtname);
	int slice_index = 0;
	for (int t = 0; t < depth; t++)
	{
		//slice_index = t * width*height * 3;
		for (int i = height - 1; i >= 0; i--) {
			slice_index = (t * slice_num + i * width)*4;
			for (int j = 0; j < width; j++) {
				f_debug1 << std::left << std::setw(4) << res1[slice_index++] << " ";
				f_debug1 << std::left << std::setw(4) << res1[slice_index++] << " ";
				f_debug1 << std::left << std::setw(4) << res1[slice_index++] << " ";
				f_debug1 << std::left << std::setw(4) << res1[slice_index++] << " ";

			}
			f_debug1 << endl;
		}
		f_debug1 << endl;
		f_debug1 << endl;
	}

	f_debug1.close();
	if (res1) delete[] res1;
}

void Image::print_buffer3(int index)
{
	float* res1 = new float[texel_num * 3];//GL_RGB
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RGB, GL_FLOAT, res1);
	string txtname = "img_lab" + std::to_string(index) + ".txt";
	ofstream f_debug1(txtname);
	int slice_index = 0;
	for (int t = 0; t < depth; t++)//depth
	{
		for (int i = height - 1; i >= 0; i--) {
			slice_index = (t * slice_num + i * width) * 3;
			for (int j = 0; j < width; j++) {
				f_debug1 << std::left << std::setw(4) << res1[slice_index++] << " ";
				f_debug1 << std::left << std::setw(4) << res1[slice_index++] << " ";
				f_debug1 << std::left << std::setw(4) << res1[slice_index++] << " ";
			}
			f_debug1 << endl;
		}
		f_debug1 << endl;
		f_debug1 << endl;
	}

	f_debug1.close();
	if (res1) delete[] res1;
}