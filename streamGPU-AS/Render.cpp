#include "Render.h"
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
Render::Render()
{
	Lloyd_ends = false;
	Lloyd_max_iter = 20;
	Lloyd_curr_iter = 0;
}

Render::~Render()
{

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Render::process_input(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		printf("next step\n");
		next_step = true;
		Sleep(1000);
	}

}

void Render::getFiles(string path, vector<string>& files, vector<string>& file_names)
{
	//long hFile = 0;//win32
	intptr_t hFile = 0;//x64
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if (!(fileinfo.attrib&_A_SUBDIR))
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
				file_names.push_back(fileinfo.name);
				//cout << fileinfo.name << endl;
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}

	string image_name;
	for (unsigned int i = 0; i < files.size(); ++i)
	{
		image_name = get_imgname(file_names[i]);
		file_names[i] = image_name;
	}
}

string Render::get_imgname(string filename_txt)
{
	string::size_type pos, _pos2;

	pos = filename_txt.find('.');
	while (string::npos != pos)
	{
		_pos2 = pos + 1;
		pos = filename_txt.find('.', _pos2);
	}
	_pos2 -= 1;
	return filename_txt.substr(0, _pos2);
}

void Render::shader_instantiate()
{
	draw_seeds.instance("../../streamGPU-AS/shaders/draw_seeds.vs", "../../streamGPU-AS/shaders/draw_seeds.fs", "../../streamGPU-AS/shaders/draw_seeds.gs");
	flood.instance("../../streamGPU-AS/shaders/flood.vs", "../../streamGPU-AS/shaders/flood.fs", "../../streamGPU-AS/shaders/flood.gs");
	centroids_next_seeds.instance("../../streamGPU-AS/shaders/centroids.vs", "../../streamGPU-AS/shaders/centroids.fs", "../../streamGPU-AS/shaders/centroids.gs");
	aver_centroids.instance("../../streamGPU-AS/shaders/aver_centroids.vs", "../../streamGPU-AS/shaders/aver_centroids.fs", "../../streamGPU-AS/shaders/aver_centroids.gs");
	aver_exception.instance("../../streamGPU-AS/shaders/aver_exception.vs", "../../streamGPU-AS/shaders/aver_exception.fs", "../../streamGPU-AS/shaders/aver_exception.gs");
	draw_next_seeds.instance("../../streamGPU-AS/shaders/draw_next_seeds.vs", "../../streamGPU-AS/shaders/draw_next_seeds.fs", "../../streamGPU-AS/shaders/draw_next_seeds.gs");
	rgb2lab.instance("../../streamGPU-AS/shaders/RGB2LAB.vs", "../../streamGPU-AS/shaders/RGB2LAB.fs", "../../streamGPU-AS/shaders/RGB2LAB.gs");
	label.instance("../../streamGPU-AS/shaders/label.vs", "../../streamGPU-AS/shaders/label.fs", "../../streamGPU-AS/shaders/label.gs");
	density_grid.instance("../../streamGPU-AS/shaders/density_grid.vs", "../../streamGPU-AS/shaders/density_grid.fs", "../../streamGPU-AS/shaders/density_grid.gs");
	derivative_t.instance("../../streamGPU-AS/shaders/derivative_t.vs", "../../streamGPU-AS/shaders/derivative_t.fs", "../../streamGPU-AS/shaders/derivative_t.gs");
	label_replace.instance("../../streamGPU-AS/shaders/label_replace.vs", "../../streamGPU-AS/shaders/label_replace.fs", "../../streamGPU-AS/shaders/label_replace.gs");
}	


int Render::run(string &dataset_path_, string &save_path_, string &dbname_, string &vdname_, int seeds_num_,
	float compactness_)
{
	// glfw: initialize and configure
   // ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	//init variables
	// -------------
	string video_path = dataset_path_;
	string save_path = save_path_;
	string dbname = dbname_;
	string vdname = vdname_;
	int seeds_num = seeds_num_;
	float compactness = compactness_;
	output_label = false;

#ifdef WIN32
	video_path = video_path + "\\" + vdname;
	save_path = save_path + dbname + "\\streamGPU-AS\\" + vdname + "\\";
	string command = "md " + save_path;
	if (_access(save_path.c_str(), 0) == -1) {
		system(command.c_str());
	}
#endif


#ifdef __linux__
	video_path = video_path + "/" + vdname;
	save_path = save_path + dbname + "/AS/" + vdname + "/";
	string commd = "mkdir -p " + save_path;
	const int dir_err = system(commd.data());
	if (-1 == dir_err)
	{
		printf("Error creating directory!n");
		exit(1);
	}

#endif
	save_path += to_string(seeds_num);
	cout << vdname << "," << seeds_num << ":" << endl;

	getFiles(video_path, files, file_names);
	sort(files.begin(), files.end());
	sort(file_names.begin(), file_names.end());
	Mat first_frame = imread(files[0], 1);
	width = first_frame.cols;
	height = first_frame.rows;
	depth_all = files.size();
	int seeds_num_whole_video = seeds_num_;
	seeds_num_100 = seeds_num_whole_video * 100 / depth_all;
	seeds_num = seeds_num_100;
	output_label = false;//set output_label=true to save txt results



	clock_t start, end;
	start = clock();

	screenWidth = width;
	screenHeight = height;
	window = glfwCreateWindow(screenWidth, screenHeight, "JFA_Test", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	const GLubyte* OpenGLVersion = glGetString(GL_VERSION);
	cout << "OpenGL version：" << OpenGLVersion << endl;
	//printf("OpenGL version：%s\n", OpenGLVersion);
	int attri_n = 0;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &attri_n);
	cout << "GL_MAX_VERTEX_ATTRIBS:" << attri_n << endl;
	glfwHideWindow(window);


	// instantiate shaders
	shader_instantiate();


	// get blocks
	block_size = 30;// 42;
	int one_third = int(block_size / 3);
	int two_third = int(2 * block_size / 3);;
	block_num = 1;
	int last_end_ind = block_size;
	vector<int> block_interval;
	bool divide_done = false;


	if (depth_all < (block_size + one_third))
	{
		block_interval.push_back(0);
		block_interval.push_back(depth_all);
		divide_done = true;
	}
	else
	{
		block_interval.push_back(0);
		block_interval.push_back(last_end_ind);
		while ((last_end_ind + two_third) < depth_all)
		{
			if (depth_all - last_end_ind - two_third < one_third)
			{
				block_interval.push_back(last_end_ind - one_third);
				block_interval.push_back(depth_all);
				divide_done = true;
			}
			else
			{
				block_interval.push_back(last_end_ind - one_third);
				block_interval.push_back(last_end_ind + two_third);
			}

			last_end_ind += two_third;
		}
		if (!divide_done)
		{
			block_interval.push_back(last_end_ind - one_third);
			block_interval.push_back(depth_all);
		}

	}
	block_num = block_interval.size() / 2;

	connected_label = false;
	labels_all = new int[width*height*depth_all];
	vector<GpuMat> gpu_frames;
	vector<Mat> flow_forward;
	vector<Mat> flow_backward;
	seeds_num_placed = 0;
	for (int i = 0; i < block_num; i++)
	{
		Lloyd_ends = false;
		Lloyd_max_iter = 20;
		Lloyd_curr_iter = 0;

		//store the final label
		int end_index = block_interval[2 * i + 1];
		int start_index = block_interval[2 * i];
		depth_block = end_index - start_index;

		Mat video_mat;
		Size video_size(width, height*depth_block);
		video_mat.create(video_size, CV_MAKETYPE(CV_8U, 3));
		video_mat = Scalar::all(0);
		Size overlap_size(width, height*one_third);
		overlap_mat.create(overlap_size, CV_MAKETYPE(CV_8U, 3));
		gpu_frames.clear();
		flow_forward.clear();
		flow_backward.clear();
		Image image;

		flow_forward.reserve(depth_block);
		flow_backward.reserve(depth_block);

		prepare_data_seeds(i, start_index, end_index, video_mat, gpu_frames, flow_forward, flow_backward, image);
		block_segment(flow_forward, flow_backward, image);
		post_and_next(i, start_index, image, video_mat);
	}
	cout << "total number of seeds:" << seeds_num_placed << endl;

	int label_num_all = 0;



	end = clock();
	double diff = (double)(end - start) / CLOCKS_PER_SEC;
	printf("ToTal Time: %f seconds\n", diff);
	//---------------------------
	//not connected in 3D
	label_num_all = seeds_num_placed;
	output_label = false;
	output_label_all(labels_all, output_label, save_path);
	output_render_all(labels_all, label_num_all);

	//---------------------------



	if (labels_all) delete labels_all;

	glfwTerminate();

	return 0;
}


void Render::forward_backward_flow(vector<GpuMat>& gpu_frames, vector<Mat>& flow_forward, vector<Mat>& flow_backward)
{
	GpuMat gpu_flow;
	gpu_flow = createContinuous(height, width, CV_32FC2);
	Ptr<cuda::DenseOpticalFlow> d_calc = cuda::FarnebackOpticalFlow::create();

	for (int i = 0; i < depth_block; i++)
	{
		if (i == 0)
		{
			d_calc->calc(gpu_frames[i], gpu_frames[i + 1], gpu_flow);
			Mat flowxy;
			gpu_flow.download(flowxy);
			flow_forward.push_back(flowxy);
			//print_flow_mat(flowxy);

			Mat flowxy2 = Mat::zeros(Size(4, 4), CV_32FC2);
			flow_backward.push_back(flowxy2);
		}
		else if (i == depth_block - 1)
		{
			Mat flowxy = Mat::zeros(Size(4, 4), CV_32FC2);
			flow_forward.push_back(flowxy);
			d_calc->calc(gpu_frames[i], gpu_frames[i - 1], gpu_flow);
			Mat flowxy2;
			gpu_flow.download(flowxy2);
			flow_backward.push_back(flowxy2);
		}
		else
		{
			d_calc->calc(gpu_frames[i], gpu_frames[i + 1], gpu_flow);
			Mat flowxy;
			gpu_flow.download(flowxy);
			flow_forward.push_back(flowxy);

			d_calc->calc(gpu_frames[i], gpu_frames[i - 1], gpu_flow);
			Mat flowxy2;
			gpu_flow.download(flowxy2);
			flow_backward.push_back(flowxy2);
		}
	}
}
void Render::prepare_data_seeds(int block_id, int start_index, int end_index, Mat& block_mat, vector<GpuMat>& gpu_frames, vector<Mat>& flow_forward, vector<Mat>& flow_backward, Image& image)
{
	cout << block_id << "(block):" << end_index-start_index << " frames, ";
	if (block_id == 0)
	{
		if(block_num>1)
			seeds_num = seeds_num_100 * (depth_block-10) / 100;//
		else
			seeds_num = seeds_num_100 * depth_block / 100;
		cout << seeds_num << " desired seeds, "<<endl;
		ac_seeds_num = seeds_num;
		Mat curr_frame;
		Mat frame_gray;
		int r_ind = start_index;
		for (int i = 0; i < depth_block; i++)
		{
			Mat frame = imread(files[r_ind++], 1);
			cvMat2TexInput(frame);
			curr_frame = block_mat(Rect(0, i*height, width, height));
			frame.copyTo(curr_frame);
			cv::cvtColor(frame, frame_gray, cv::COLOR_RGB2GRAY);
			gpu_frames.push_back(GpuMat(frame_gray));
		}
		/*string name = "video_block.txt";
		output_video(block_mat.data, width, height, depth_block, name);*/

		forward_backward_flow(gpu_frames, flow_forward, flow_backward);
		seeds_lsc_adjust.clear();
		image.init_gpu(block_mat.data, width, height, depth_block, seeds_num, Lloyd_max_iter);
		image.initialization_grid(ac_seeds_num);
		cout << "kept+new seeds:"<< ac_seeds_num << endl;
		image.initialization_adjust(derivative_t, density_grid);
		image.get_ac_seeds(seeds_lsc_adjust);
		image.init_variables_attributes(ac_seeds_num);

		map<int, int> block2all;
		for (int i = 0; i < ac_seeds_num; i++)
		{
			block2all.insert(pair<int, int>(i, int(seeds_lsc_adjust[i * 4 + 3])));
		}

		block2all_vec.push_back(block2all);
	
	}
	else
	{
		seeds_num = seeds_num_100 * (depth_block-10)/ 100;
		cout << seeds_num << " desired seeds,";
		kept_seeds_num = kept_seeds.size() / 4;
		//seeds_num = seeds_num - kept_seeds_num;
		ac_seeds_num = seeds_num;
		int one_third = int(block_size / 3);
		//string name = "overlap.txt";
		//output_video(overlap_mat.data, width, height, one_third, name);
		
		Mat curr_frame;
		Mat frame_gray;
		curr_frame = block_mat(Rect(0, 0, width, one_third*height));
		overlap_mat.copyTo(curr_frame);
		for (int i = 0; i < one_third; i++)
		{
			Mat frame = block_mat(Rect(0, i*height, width, height));
			cv::cvtColor(frame, frame_gray, cv::COLOR_RGB2GRAY);
			gpu_frames.push_back(GpuMat(frame_gray));
		}
		
		int r_ind = start_index + one_third;
		for (int i = one_third; i < depth_block; i++)
		{
			Mat frame = imread(files[r_ind], 1);
			cvMat2TexInput(frame);
			curr_frame = block_mat(Rect(0, i*height, width, height));
			frame.copyTo(curr_frame);
			cv::cvtColor(frame, frame_gray, cv::COLOR_RGB2GRAY);
			gpu_frames.push_back(GpuMat(frame_gray));
			r_ind++;
		}
		/*string name = "block.txt";
		output_video(block_mat.data, width, height, depth_block, name);*/

		forward_backward_flow(gpu_frames, flow_forward, flow_backward);
		image.init_gpu(block_mat.data, width, height, depth_block, seeds_num, Lloyd_max_iter);
		image.initialization_grid(ac_seeds_num);
		
		image.initialization_adjust(derivative_t,density_grid);

		vector<float> seeds_added;
		image.get_ac_seeds(seeds_added);
		int added_id = seeds_num_placed;
		seeds_lsc_adjust.clear();
		seeds_lsc_adjust.insert(seeds_lsc_adjust.begin(),kept_seeds.begin(), kept_seeds.end());
		seeds_lsc_adjust.insert(seeds_lsc_adjust.begin() + (kept_seeds_num*4), seeds_added.begin(), seeds_added.end());//注意：这里kept_seeds_num*4
		ac_seeds_num = seeds_lsc_adjust.size() / 4;//update ac_seeds_num, and tell image
		cout << "kept+new seeds:" << ac_seeds_num << endl;
		
		map<int, int> block2all0;
		int kept_sind_last_block=0;
		int kept_sind_all = 0;
		for (int i = 0; i < ac_seeds_num; i++)
		{
			if (i < kept_seeds_num)
			{
				kept_sind_last_block = int(kept_seeds[i * 4 + 3]);
				kept_sind_all = block2all_vec[block_id - 1][kept_sind_last_block];
				block2all0.insert(pair<int, int>(i, kept_sind_all));
			}
			else
			{
				block2all0.insert(pair<int, int>(i, added_id++));
			}
			//correct seed id
			seeds_lsc_adjust[i * 4 + 3] = i;
		}
		image.init_variables_attributes(ac_seeds_num);

		block2all_vec.push_back(block2all0);
		
	}
	

}

void Render::block_segment(vector<Mat>& flow_forward, vector<Mat>& flow_backward,Image& image)
{
	image.seed_matrix(flow_forward, flow_backward,seeds_lsc_adjust);
	image.RGB2LAB(rgb2lab);

	Lloyd_curr_iter = 0;
	while (!Lloyd_ends)
	{
		Lloyd_curr_iter++;
		if (Lloyd_curr_iter == 1)
		{
			image.Ld_first_init(draw_seeds);
		}
		else
		{
			//image.Ld_next_init(draw_next_seeds,Lloyd_curr_iter);
			image.Ld_next_seeds_matrix(draw_next_seeds, flow_forward, flow_backward, Lloyd_curr_iter);//在这个函数里调用了split_step2();
		}

		image.flood(flood, Lloyd_curr_iter);


		if (Lloyd_curr_iter == Lloyd_max_iter)
		{
			Lloyd_ends = true;
			image.centroids_next_seeds(centroids_next_seeds, aver_exception, Lloyd_curr_iter);
		}
		else
		{
			image.centroids_next_seeds(centroids_next_seeds, aver_exception, Lloyd_curr_iter);
		}

	}

}


void Render::post_and_next(int block_id, int start_index,Image& image, Mat& block_mat)
{
	int pixel_num_block = width * height*depth_block;
	int* labels = new int[pixel_num_block];
	vector<float> seeds_info;//x,y,z,id
	image.check_dead_seeds(label, label_replace,labels, seeds_info);
	int final_seeds_num = int(seeds_info.size() / 4);
	seeds_num_placed += (final_seeds_num-kept_seeds_num);
	int one_third = int(block_size / 3);
	int two_third = block_size - one_third;
	int done_pixel_num = pixel_num_block;
	int done_frames = depth_block;
	if (block_id < block_num - 1)
	{
		int sind = 0; float seed_z = 0, next_z = 0;
		kept_seeds_num = 0;
		kept_seeds.clear();
		for (int i = 0; i < final_seeds_num; i++)
		{
			seed_z = seeds_info[sind + 2];
			if (seed_z > (block_size - one_third))
			{
				kept_seeds.push_back(seeds_info[sind]);
				kept_seeds.push_back(seeds_info[sind + 1]);
				next_z = seed_z - two_third;
				if (next_z < 1)
					next_z = 1.5;
				kept_seeds.push_back(next_z);
				kept_seeds.push_back(seeds_info[sind + 3]);
				kept_seeds_num++;
			}
			
			sind += 4;
		}

		cout << "block done! Kept seeds num: " << kept_seeds_num << endl;
		cout << "placed seeds num so far: " << seeds_num_placed << endl;

		done_pixel_num = width * height*(depth_block - one_third);
		done_frames = depth_block - one_third;
		
		int done_f_ind = block_size - one_third;//20
		Mat overlap_frame;
		Mat curr_frame;
		for (int i = 0; i < one_third; i++)
		{
			curr_frame = block_mat(Rect(0, (done_f_ind + i)*height, width, height));
			overlap_frame = overlap_mat(Rect(0, i*height, width, height));
			curr_frame.copyTo(overlap_frame);
		}
	}

	int start_pid = start_index * height*width;
	for (int i = 0; i < done_pixel_num; i++)
	{
		labels_all[start_pid++] = block2all_vec[block_id][labels[i]];
	}

	
	image.unload();
	
	if (labels) delete[] labels;
}


void Render::label_connected3D(int* label_all, int* labels_connected, int& label_num_all, int SUPSZ)
{
	int texel_num = width * height*depth_all;
	int slice_num = width * height;

	const int dx8[8] = { -1, -1,  0,  1, 1, 1, 0, -1 };
	const int dy8[8] = { 0, -1, -1, -1, 0, 1, 1,  1 };
	const int dz3[3] = { -1,0,1 };

	//nlabels.resize(sz, -1);
	for (int i = 0; i < texel_num; i++)
	{
		labels_connected[i] = -1;
	}
	int label(0);//index of superpixel starts from 0！！！！！！！！！！！！！
	//seed_range = 10*pow(seed_range, 3);
	int* xvec = new int[texel_num];
	int* yvec = new int[texel_num];
	int* zvec = new int[texel_num];
	int oindex(0);
	int adjlabel(0);//adjacent label
	int x = 0, y = 0, z = 0;
	bool merge_flag = false;
	for (int i = 0; i < depth_all; i++)
	{
		for (int j = 0; j < height; j++)
		{
			for (int k = 0; k < width; k++)
			{
				if (0 > labels_connected[oindex])
				{
					labels_connected[oindex] = label;
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
						if (z >= 0 && z < depth_all)
						{
							for (int n = 0; n < 8; n++)
							{
								x = xvec[0] + dx8[n];
								y = yvec[0] + dy8[n];
								if ((x >= 0 && x < width) && (y >= 0 && y < height))
								{
									int nindex = z * slice_num + y * width + x;
									if (labels_connected[nindex] >= 0) adjlabel = labels_connected[nindex];
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

							if (z >= 0 && z < depth_all)
							{
								for (int n = 0; n < 8; n++)
								{
									x = xvec[c] + dx8[n];
									y = yvec[c] + dy8[n];

									if ((x >= 0 && x < width) && (y >= 0 && y < height))
									{
										int nindex = z * slice_num + y * width + x;
										if (0 > labels_connected[nindex] && (label_all[oindex] == label_all[nindex]))
										{
											xvec[count] = x;
											yvec[count] = y;
											zvec[count] = z;
											labels_connected[nindex] = label;
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
					if (count < (SUPSZ / 4))//>> 2表示除以4,SUPSZ >> 3
					{
						for (int c = 0; c < count; c++)
						{
							int ind = zvec[c] * slice_num + yvec[c] * width + xvec[c];
							labels_connected[ind] = adjlabel;
						}
						label--;
					}




					label++;
				}
				oindex++;
			}
		}
	}

	label_num_all = label;






}
void  Render::cvMat2TexInput(Mat& img)
{
	GaussianBlur(img, img, Size(3, 3), 0);
	cvtColor(img, img, COLOR_BGR2RGB);
	//cvtColor(img, img, COLOR_BGR2Lab);
	flip(img, img, 0);//rows-i-1,j
}


void Render::output_video(unsigned char* data, int frame_width, int frame_height, int depth, string name)
{
	string txtname = name;
	ofstream f_debug1(txtname);
	int pi = 0;

	for (int i = 0; i < depth * frame_height; i++) {
		for (int j = 0; j < frame_width; j++) {
			f_debug1 << std::left << std::setw(4) << int(data[pi++]) << " ";
			f_debug1 << std::left << std::setw(4) << int(data[pi++]) << " ";
			f_debug1 << std::left << std::setw(4) << int(data[pi++]) << " ";
		}
		f_debug1 << endl;
		if ((i + 1) % frame_height == 0)
			f_debug1 << endl << endl;
	}
	f_debug1.close();
}

void Render::output_frame(unsigned char* data, int index, int width, int height)
{
	string txtname = "frame" + std::to_string(index) + ".txt";
	ofstream f_debug1(txtname);
	int pi = 0;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			f_debug1 << std::left << std::setw(4) << int(data[pi++]) << " ";
			f_debug1 << std::left << std::setw(4) << int(data[pi++]) << " ";
			f_debug1 << std::left << std::setw(4) << int(data[pi++]) << " ";
		}
		f_debug1 << endl;
	}
	f_debug1.close();
}


void Render::output_label_all(int* label_all, bool output_label, string& save_path)
{
	if (output_label)
	{
		string txtname = save_path + ".txt";
		int slice_num = height * width;
		ofstream f_debug1(txtname);
		int slice_index = 0;
		for (int t = 0; t < depth_all; t++)
		{
			for (int i = height - 1; i >= 0; i--) {
				slice_index = t * slice_num + i * width;
				for (int j = 0; j < width; j++) {
					f_debug1 << std::left << std::setw(4) << int(label_all[slice_index]) << " ";//注意这里之前输出res了，处理离散最后应该输出labels!!!
					slice_index++;
				}
				f_debug1 << endl;
			}
			f_debug1 << endl;
			f_debug1 << endl;
		}
		f_debug1.close();
		cout << "save not-connected labels_all for video !!!" << endl;
	}

}

void Render::output_render_all(int* label_all, int seeds_all)
{
	Mat render(Size(width, height*depth_all), CV_8UC3, Scalar(0, 0, 0));
	unsigned char* data = render.data;
	int* render_color = new int[seeds_all * 3];
	for (int i = 0; i < seeds_all * 3; i++)
	{
		render_color[i] = rand() % 256;
	}

	int label_index = 0;
	int label_va = 0;
	uchar* ptr = NULL;
	for (int i = 0; i < (depth_all*height); i++)
	{
		ptr = render.ptr<uchar>(i);
		for (int j = 0; j < width; j++)
		{
			label_va = 3 * label_all[label_index++];
			if (label_va < 0)
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


	int slice_num = width * height;
	unsigned char* slice_start = NULL;
	for (int i = 0; i < depth_all; i++)
	{
		if (i == 0)
			slice_start = render.data;
		else
			slice_start = slice_start + 3 * slice_num;

		Mat seg(Size(width, height), CV_8UC3, slice_start);
		cvtColor(seg, seg, COLOR_BGR2RGB);
		flip(seg, seg, 0);//rows-i-1,j
		imshow("JFAInitMatrix", seg);
		waitKey();

		/*char savepath[1024];
		sprintf_s(savepath, "voxel\\test\\%05d_%05d.png", start_index, (i + start_index + 1));
		imwrite(savepath, seg);*/
	}


	if (render_color) delete[] render_color;
}

