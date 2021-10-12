#pragma once
#ifndef OPENCV_OPENGL_TEST_RENDER_H_S
#define OPENCV_OPENGL_TEST_RENDER_H_S

#endif //OPENCV_OPENGL_TEST_RENDER_H_S
#include<io.h>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include<iomanip>
#include <vector>
#include "Image.h"


using namespace std;
using namespace cv;

class Render
{
private:
	GLFWwindow * window;
	int width = 146;
	int height = 120;
	int depth_all = 25;
	int depth_block = 25;
	bool output_label;
	int seeds_num;
	int ac_seeds_num;
	int seeds_num_100;
	bool split_flag;
	string dbname;
	string vdname;
	int screenWidth, screenHeight;
	vector<float> seeds_lsc_adjust;
	int block_num;
	vector<string> files;
	vector<string> file_names;
	vector<float> kept_seeds;
	int kept_seeds_num;
	int seeds_num_placed;
	int block_size;
	Mat overlap_mat;
	int* labels_all;
	vector<map<int, int>> block2all_vec;
	bool connected_label;

	Shader draw_seeds;
	Shader flood;
	Shader centroids_next_seeds;
	Shader aver_centroids;
	Shader aver_exception;
	Shader draw_next_seeds;
	Shader rgb2lab;
	Shader label;
	Shader density_grid;
	Shader derivative_t;
	Shader label_replace;
	void shader_instantiate();


	string get_imgname(string filename_txt);
	void getFiles(string path, vector<string>& files, vector<string>& file_names);
	//void seeds_grid(Image& image);
	void forward_backward_flow(vector<GpuMat>& gpu_frames, vector<Mat>& flow_forward, vector<Mat>& flow_backward);
	void prepare_data_seeds(int block_id, int start_index, int end_index, Mat& block_mat, vector<GpuMat>& gpu_frames, vector<Mat>& flow_forward, vector<Mat>& flow_backward, Image& image);
	void block_segment(vector<Mat>& flow_forward, vector<Mat>& flow_backward, Image& image);
	void post_and_next(int block_id, int start_index, Image& image, Mat& block_mat);
public:
	bool next_step;
	bool Lloyd_ends;
	int Lloyd_curr_iter;
	int Lloyd_max_iter;
	Render();
	~Render();

	int run(string &dataset_path_, string &save_path_, string &dbname_, string &vdname_, int seeds_num_,
		float compactness_);
	void cvMat2TexInput(Mat& img);
	void process_input(GLFWwindow *window);
	void output_video(unsigned char* data, int frame_width, int frame_height, int depth, string name);
	void output_frame(unsigned char* data, int index, int width, int height);
	void output_label_all(int* label_all, bool output_label, string& name);
	void label_connected3D(int* label_all, int* labels_connected,int& label_num_all,int SUPSZ);
	void output_render_all(int* label_all,int seeds_all);
	
}; 
