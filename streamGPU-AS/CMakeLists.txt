cmake_minimum_required(VERSION 2.8)
project(streamGPU-AS)
set(CMAKE_CXX_STANDARD 11)
aux_source_directory(. SRC_LIST)
if(WIN32)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
    include_directories(${CMAKE_SOURCE_DIR}/includes)
	include_directories(${CMAKE_SOURCE_DIR}/includes/eigen)# download eigen, and put it in the "includes" dir.
    link_directories(${CMAKE_SOURCE_DIR}/lib)
    find_package(glfw3 REQUIRED)
    set(OpenCV_DIR D:/workspace_vs17/glfw_glad_opencv/opencv_4_0_0_cuda_10_0_mkl_tbb/install/x64/vc15/lib)
    find_package(OpenCV REQUIRED)
    file(GLOB project_file glad.c main.cpp)
    add_executable(${PROJECT_NAME} ${SRC_LIST} Render.cpp Render.h Image.h Image.cpp)
    target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${GLFW3_LIBRARY})
elseif(UNIX AND NOT APPLE)
    find_package(glfw3 REQUIRED)
    file(GLOB project_file glad.c main.cpp)
    add_executable(${PROJECT_NAME} ${SRC_LIST} Render.cpp Render.h Image.h Image.cpp)
    find_package( OpenCV REQUIRED )
    target_link_libraries(${PROJECT_NAME}
            ${OpenCV_LIBS}
            -lglfw3
            -lGL
            -lm
            -lXrandr
            -lXi
            -lX11
            -lXxf86vm
            -lpthread
            -ldl
            )
endif(WIN32)