#pragma once
#ifndef OPENCV_OPENGL_TEST_IMAGE_H_S
#define OPENCV_OPENGL_TEST_IMAGE_H_S

#endif //OPENCV_OPENGL_TEST_IMAGE_H_S
#include <iostream>
#include <vector>
#include <glad/glad.h>
#include "shader.h"
#include <GLFW/glfw3.h>
#include <iomanip>
#include <map>
#include <opencv2/core/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/cudaobjdetect.hpp>
#include <opencv2/cudaoptflow.hpp>
#include <opencv2/core/opengl.hpp>
#include <Eigen/Dense>
#include <glad/glad.h>




using namespace std;
using namespace cv;
using namespace cuda;
using namespace Eigen;
class Image
{
private:
	
	//float *texels;

	int texel_num;
	int slice_num;
	int width, height, depth;

	float width_f, height_f, depth_f;
	float width_ratio, height_ratio, depth_ratio;
	int seeds_num;
	vector<float> seeds_lsc;
	vector<float> seeds_lsc_adjust;
	int final_label_num;
	int ac_seeds_num;
	int dNum, wNum, hNum;
	
	int current_buffer;
	int Lloyd_max_iter;
	int s;
	int max_s;
	int seed_range;
	float compactness;
	float ratio;
	float step_length;
	int flood_iter;
	GLuint frameBuffer_image;
	GLuint texture_image[4];
	GLenum buffers_image[4];
	GLuint frameBuffer_flood;
	GLuint texture_flood[5];
	GLenum buffers_flood[5];

	int seed_color_index;
	unsigned int VBO_s, VAO_s;
	unsigned int VBO_all_t, VAO_all_t;
	unsigned int VBO_s_index, VAO_s_index;
	//unsigned int VBO_test, VAO_test;
	map<int, int, greater<int>> gradient_clusterId;
public:
	
	Image();
	~Image();
	void get_ac_seeds(vector<float>& seeds_all);
	void initialization_adjust(Shader& derivative_t,Shader& density_grid);
	void initialization_grid(int& ac_seeds_num_);
	bool init_gpu(unsigned char* data, int width,int height, int depth, int seeds_num,int Lloyd_max_iter);
	void RGB2LAB(Shader& rgb2lab);
	void Ld_first_init(Shader& draw_seeds);
	void flood(Shader& flood,int Lloyd_curr_iter);
	void centroids_next_seeds(Shader& centroids_next_seeds, Shader& aver_exception,int Lloyd_curr_iter);
	void Ld_next_seeds_matrix(Shader& draw_next_seeds,vector<Mat>& flow_forward, vector<Mat>& flow_backward, int Lloyd_curr_iter);
	void print_seg(unsigned char* data);
	void check_dead_seeds(Shader& label, Shader& label_replace, int* labels, vector<float>& seeds_info);

	void seed_matrix(vector<Mat>& flow_forward, vector<Mat>& flow_backward, vector<float>& seeds_lsc_adjust_);
	bool matrix_slover(float seed_x,float seed_y,int on_frame_index, vector<Mat>& flow_forward, vector<Mat>& flow_backward,vector<float>& matrix_6_paras);
	void enforce_connectivity_3d(Shader& label, int* labels, int& final_label_num,string dirName,bool output_label);
	void label_notconnected(int* labels, string dirName, bool output_label);

	void print_connect_seg(unsigned char* data, int* labels);
	void print_render(int* labels, int start_index);
	void init_variables_attributes(int& ac_seeds_num_);
	void unload();
private:
	bool init_flood_texture();
	bool init_image_texture(unsigned char* data);
	void output_3d();
 
	void reset_variables();
	void print_flow_mat(Mat& flow);
	void print_image_label();
	void print_image_label_float();
	void print_final(int index);
	void print_buffer(int index);
	void print_buffer3(int index);
	void print_buffer_1float(int index);

	void EnforceLabelConnectivity3D(
		int*					labels,//input labels that need to be corrected to remove stray labels
		int*&						nlabels,//new labels
		int&						numlabels,//the number of labels changes in the end if segments are removed
		const int&					K);
	


	void sequence_label(int* labels, map<int, int>& sequence_label_map, bool dead_seed);
};
