#pragma once
#ifndef OPENCV_OPENGL_TEST_RENDER_H
#define OPENCV_OPENGL_TEST_RENDER_H

#endif //OPENCV_OPENGL_TEST_RENDER_H
#ifdef __linux__
#include <dirent.h>
#endif
#ifdef WIN32
#include<io.h>
#endif

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
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

    int screenWidth, screenHeight;
    string get_imgname(string filename_txt);
    void getFiles(string path, vector<string>& files, vector<string>& file_names);
public:
    bool next_step;
    bool Lloyd_ends;
    int Lloyd_curr_iter;
    int Lloyd_max_iter;
	int width;
	int height;
	int depth;
	bool output_label;

    Render();
    ~Render();

    int run(string& dataset_path_, string& save_path_,string& dbname_, string& vdname_, int seeds_num_,float compactness_);
    void cvMat2TexureInput(Mat& img);
    void process_input(GLFWwindow *window);
};
