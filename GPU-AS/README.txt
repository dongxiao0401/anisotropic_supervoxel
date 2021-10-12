This is the source code that generates GPU-based Anisotropic Supervoxel, which is presented in the IEEE TIP Submission:

GPU-based Supervoxel Generation with a Novel Anisotropic Metric

We provide the source code that tested on Windows and Ubuntu.

==========================================
##Runtime Environment##
The project requires: 
OpenGL 4.2 or later;
Eigen library;
OpenCV with GPU support. 


On Windows: 
The GLFW3 and Eigen files are provided in the project, you need install OpenCV with Cuda, configure the project by CMakeLists and specify the "OpenCV_DIR" in CMakeLists.txt.

On Linux:
install OpenCV with Cuda, GLFW, GLAD, GLM, Eigen, configure the project by CMakeLists.


==========================================
##Parameters##

Specify the paths of input dataset and the output file in main.cpp.

The program generates supervoxel segmentation results, including txt files and pseudo-color images. 

Download the video datasets (SegTrackv2, BuffaloXiph) provided in LIBSVXv4.0:
http://www.cs.rochester.edu/~cxu22/d/libsvx/
