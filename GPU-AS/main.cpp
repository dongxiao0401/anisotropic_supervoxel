#include "Render.h"
#include <stdio.h>
#ifdef __linux__
#include <dirent.h>
#include <unistd.h>
#endif
using namespace std;


#ifdef __linux__
void getFileNames(string path, vector<string>& file_names)
{
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
}
#endif
#ifdef WIN32
void getFileNames(string path, vector<string>& file_names)
{
	intptr_t hFile = 0;
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib&_A_SUBDIR))
			{
				if (fileinfo.name[0] != '.')
					file_names.push_back(fileinfo.name);
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}
#endif


int main() {
    string dbname = "SegTrackv2";
    vector<string> files_B;
    vector<string> file_names_B;
    string dataset_path;
    string save_path;
    float compactness = 8;
    vector<int> svnum;
    svnum.push_back(500);
    
    #ifdef __linux__
    dataset_path = "../../Datasets/" + dbname + "/PNGImages";
    save_path = "../../Datasets/results/";
    char* curr_path = getcwd(NULL,0);
    cout<<curr_path<<endl;
    getFileNames(dataset_path, file_names_B);
    #endif
#ifdef WIN32
	//specify the input path of videos
    dataset_path = "D:\\workspace_vs17\\datasets\\" + dbname + "\\PNGImages";
	//specify the output path of supervoxel results, set output_label=true
	save_path = "D:\\workspace_vs17\\JFA-supervoxel-project\\anisotropic_supervoxel\\GPU-AS\\results\\";
	getFileNames(dataset_path, file_names_B);
#endif
        int video_number = file_names_B.size();
        for (int i = 0; i < video_number; i++)
        {
            string vdname = file_names_B[i];
            for (int j = 0; j < svnum.size(); j++)
            {
                int seed_num = svnum[j];
                Render render;
                render.run(dataset_path,save_path, dbname, vdname, seed_num,compactness);
            }
        }
        system("pause");
        return 0;
}


