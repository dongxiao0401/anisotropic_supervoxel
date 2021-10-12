#include "Render.h"
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <sys/stat.h> 　
#include <sys/types.h>
#include  <direct.h>
#include  <stdio.h>
Render::Render() {
    Lloyd_ends = false;
    Lloyd_max_iter = 20;
    Lloyd_curr_iter = 0;
}

Render::~Render() {

}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Render::process_input(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        printf("next step\n");
        next_step = true;
#ifdef __linux__
        sleep(1000);
#elif defined(WIN32)
		Sleep(1000);
#endif
    }
}

void Render::getFiles(string path, vector<string> &files, vector<string> &file_names) {
#ifdef __linux__
    DIR *directory_pointer;
    struct dirent *entry;
    if((directory_pointer=opendir(path.c_str()))==NULL){
        printf("Error open\n");
        return ;
    } else {
        while((entry=readdir(directory_pointer))!=NULL){
            if(entry->d_name[0]=='.') continue;
            //printf("%s\n",entry->d_name);
            file_names.push_back(entry->d_name);
        }
    }

    for (unsigned int i = 0; i < file_names.size(); ++i) {
        string img_path = path+'/'+file_names[i];
        files.push_back(img_path);
    }
#endif
#ifdef WIN32
    //long hFile = 0;//win32
    intptr_t hFile = 0;//x64
    struct _finddata_t fileinfo;
    string p;
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
        do {
            if (!(fileinfo.attrib & _A_SUBDIR)) {
                files.push_back(p.assign(path).append("\\").append(fileinfo.name));
                file_names.push_back(fileinfo.name);
            }
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }
    /*string img_path;
    for (unsigned int i = 0; i < file_names.size(); ++i) {
        img_path = path+'\\'+file_names[i];
        files[i] = img_path;
    }*/
#endif

}

string Render::get_imgname(string filename_txt) {
    string::size_type pos, _pos2;

    pos = filename_txt.find('.');
    while (string::npos != pos) {
        _pos2 = pos + 1;
        pos = filename_txt.find('.', _pos2);
    }
    _pos2 -= 1;
    return filename_txt.substr(0, _pos2);
}

int Render::run(string &dataset_path_, string &save_path_, string &dbname_, string &vdname_, int seeds_num_,
                float compactness_) {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,4);
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
    width = 146;
    height = 120;
    depth = 25;
    output_label = false;

#ifdef WIN32
    video_path = video_path + "\\" + vdname;
    save_path = save_path + dbname + "\\GPU-AS\\" + vdname + "\\";
    string command = "md " + save_path;
    if (_access(save_path.c_str(), 0) == -1) {
        system(command.c_str());
    }
#endif


#ifdef __linux__
    video_path = video_path + "/" + vdname;
    save_path = save_path + dbname + "/AS/" + vdname + "/";
    string commd= "mkdir -p "+save_path;
    const int dir_err = system(commd.data());
    if (-1 == dir_err)
    {
        printf("Error creating directory!n");
        exit(1);
    }

#endif
    save_path += to_string(seeds_num);
    cout << vdname << "," << seeds_num << ":" << endl;

    vector<string> files;
    vector<string> file_names;
    getFiles(video_path, files, file_names);
    sort(files.begin(), files.end());
    sort(file_names.begin(), file_names.end());
    Mat first_frame = imread(files[0], 1);//get width and height
    width = first_frame.cols;
    height = first_frame.rows;
    depth = file_names.size();

    screenWidth = width;
    screenHeight = height;
    window = glfwCreateWindow(screenWidth, screenHeight, "JFA_Test", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    const GLubyte *OpenGLVersion = glGetString(GL_VERSION);
    cout << "OpenGL version: " << OpenGLVersion << endl;
    int attri_n = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &attri_n);
    cout << "GL_MAX_VERTEX_ATTRIBS: " << attri_n << endl;
	glfwHideWindow(window);
    //load shaders
    //------------
#ifdef __linux__
	/*char *buffer;
	buffer = getcwd(NULL, 0);
	cout << "current path: " << buffer << endl;*/
#endif

	/*char   buffer[MAX_PATH];
	getcwd(buffer, MAX_PATH);
	printf("The   current   directory   is:   %s ", buffer);*/
	

    Shader draw_seeds = Shader("../../GPU-AS/shaders/draw_seeds.vs", "../../GPU-AS/shaders/draw_seeds.fs", "../../GPU-AS/shaders/draw_seeds.gs");
    Shader flood = Shader("../../GPU-AS/shaders/flood.vs", "../../GPU-AS/shaders/flood.fs", "../../GPU-AS/shaders/flood.gs");
    Shader centroids_next_seeds = Shader("../../GPU-AS/shaders/centroids.vs", "../../GPU-AS/shaders/centroids.fs", "../../GPU-AS/shaders/centroids.gs");
    Shader aver_exception = Shader("../../GPU-AS/shaders/aver_exception.vs", "../../GPU-AS/shaders/aver_exception.fs", "../../GPU-AS/shaders/aver_exception.gs");
    Shader draw_next_seeds = Shader("../../GPU-AS/shaders/draw_next_seeds.vs", "../../GPU-AS/shaders/draw_next_seeds.fs", "../../GPU-AS/shaders/draw_next_seeds.gs");
    Shader rgb2lab = Shader("../../GPU-AS/shaders/RGB2LAB.vs", "../../GPU-AS/shaders/RGB2LAB.fs", "../../GPU-AS/shaders/RGB2LAB.gs");
    Shader label = Shader("../../GPU-AS/shaders/label.vs", "../../GPU-AS/shaders/label.fs", "../../GPU-AS/shaders/label.gs");
    Shader derivative_t = Shader("../../GPU-AS/shaders/derivative_t.vs", "../../GPU-AS/shaders/derivative_t.fs", "../../GPU-AS/shaders/derivative_t.gs");
    Shader density_grid = Shader("../../GPU-AS/shaders/density_grid.vs", "../../GPU-AS/shaders/density_grid.fs", "../../GPU-AS/shaders/density_grid.gs");
	Shader label_replace = Shader("../../GPU-AS/shaders/label_replace.vs", "../../GPU-AS/shaders/label_replace.fs", "../../GPU-AS/shaders/label_replace.gs");



    time_t start = 0, end = 0;
    double time_ms = 0;
    time_t start1 = 0, end1 = 0;
    double time_ms1 = 0;
    start1 = clock();

    //prepare data: video_mat, gpu_frames
    //-----------------------------------
    Mat curr_frame;
    Mat video_mat;
    Size video_size(width, height * depth);
    video_mat.create(video_size, CV_MAKETYPE(CV_8U, 3));
    video_mat = Scalar::all(0);
    cvMat2TexureInput(first_frame);
    curr_frame = video_mat(Rect(0, 0, width, height));
    first_frame.copyTo(curr_frame);

    vector<GpuMat> gpu_frames;
    Mat frame_gray;
    cv::cvtColor(first_frame, frame_gray, cv::COLOR_RGB2GRAY);
    gpu_frames.push_back(GpuMat(frame_gray));
    vector<Mat> flow_forward;
    vector<Mat> flow_backward;
    flow_forward.reserve(depth);
    flow_backward.reserve(depth);
    for (int i = 1; i < depth; i++)//read from 1
    {
        Mat frame = imread(files[i], 1);
        cvMat2TexureInput(frame);
        curr_frame = video_mat(Rect(0, i * height, width, height));
        frame.copyTo(curr_frame);
        cv::cvtColor(frame, frame_gray, cv::COLOR_RGB2GRAY);//check image format of cvMat2TexureInput, RGB or BGR
        gpu_frames.push_back(GpuMat(frame_gray));
    }

    end1 = clock();
    time_ms1 = (double) (end1 - start1) * 1000 / CLOCKS_PER_SEC;
    cout << "load data, time_ms1:" << time_ms1 << endl;
    //output_video(video_mat.data, width, height);

    //video segmentation start
    //------------------------
    start = clock();
	int* labels = new int[width*height*depth];
    Image image;
    //1. prepare GPU texture, initialize seeds as grids
    image.init_gpu(dbname, video_mat.data, width, height, depth, seeds_num, Lloyd_max_iter, compactness);
    //2. adjust seed initialization based on object richness
    image.initialization(derivative_t, density_grid);

    time_t start_f = 0, end_f = 0;
    start_f = clock();
    //3. compute optical flow fields, and the anisotropic matrices of seeds.
    image.forward_backward_flow(gpu_frames, flow_forward, flow_backward);
    end_f = clock();
    time_ms = (double) (end_f - start_f) * 1000 / CLOCKS_PER_SEC;
    cout << "forward backward optical flow, time_ms:" << time_ms << endl;
    start_f = clock();
    image.seed_matrix(flow_forward, flow_backward);
    end_f = clock();
    time_ms = (double) (end_f - start_f) * 1000 / CLOCKS_PER_SEC;
    cout << "seed_matrix, time_ms:" << time_ms << endl;

    //4. RGB space to LAB space
    image.RGB2LAB(rgb2lab);

    //5. Optimization: Lloyd iteration
    Lloyd_curr_iter = 0;
    while (!Lloyd_ends) {
        Lloyd_curr_iter++;
        if (Lloyd_curr_iter == 1) {
            image.Ld_first_init(draw_seeds);
        } else {
            image.Ld_next_seeds_matrix(draw_next_seeds, flow_forward, flow_backward,
                                       Lloyd_curr_iter);
        }
        image.flood(flood, Lloyd_curr_iter);
		if (Lloyd_curr_iter == Lloyd_max_iter) {
			Lloyd_ends = true;
		}
       
		image.centroids_next_seeds(centroids_next_seeds, aver_exception, Lloyd_curr_iter);

    }

	
    /*
	  6.1 post-processing:
	  check label continuity(some seeds died during iteration); check label valid(some pixels' label are -1)
	  We do not enforce connectivity in evaluation experiments.
	  (use 6.1 or 6.2 to generate results)
	*/
    start1 = clock();
    image.check_dead_seeds(label, label_replace, labels);
    end1 = clock();
    time_ms = (double) (end1 - start1) * 1000 / CLOCKS_PER_SEC;
    cout << "check_dead_seeds, time_ms:" << time_ms << endl;
    cout << "done!" << endl;
    start1 = clock();
    //output_label=true to print txt result.
    output_label = false;
    image.print_sv_txt(labels, save_path, output_label,false);
    end1 = clock();
    time_ms = (double) (end1 - start1) * 1000 / CLOCKS_PER_SEC;
    cout << "save txt, time_ms:" << time_ms << endl;
    end = clock();
    time_ms = (double) (end - start) / CLOCKS_PER_SEC;
    cout << "video segmentation done, total time(ms):" << time_ms << endl;
    //uncomment it to show the supervoxel result
    image.print_render(labels);
	//image.print_seg(video_mat.data,labels);


 //   /*
	//  6.2 post-processing: enforce connectivity in 3D space
 //     It works better in application with 3D connectivity.
 //   */
 //   start1 = clock();
 //   int label_num = 0;
 //   //enforce connectivity and generate txt result.
 //   image.enforce_connectivity_3d(label, labels, label_num);
	//output_label = false;
	//image.print_sv_txt(labels, save_path, output_label, true);
 //   cout << "label_num=" << label_num << endl;
 //   end1 = clock();
 //   time_ms1 = (double)(end1 - start1) * 1000 / CLOCKS_PER_SEC;
 //   cout << "enforce connectivity, time:" << time_ms1 << endl;
 //   end = clock();
 //   time_ms = (double)(end - start) * 1000 / CLOCKS_PER_SEC;
 //   cout << "video segmentation done, total time(ms):" << time_ms << endl;
 //   image.print_render(labels);//show the supervoxel
	////image.print_connect_seg(video_mat.data, labels);




    if (labels) delete[] labels;
    glfwTerminate();

    return 0;
}

//change image format in opencv to the format in opengl
//-----------------------------------------------------
void Render::cvMat2TexureInput(Mat &img) {
    GaussianBlur(img, img, Size(3, 3), 0);
    cvtColor(img, img, COLOR_BGR2RGB);
    //cvtColor(img, img, COLOR_BGR2Lab);
    flip(img, img, 0);//rows-i-1,j
}

