#include "Render.h"

string get_imgname(string filename_txt)
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


void getFileNames(string path, vector<string>& file_names)
{
	//读取目录
	intptr_t hFile = 0;
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录
			if ((fileinfo.attrib&_A_SUBDIR))
			{
				//files.push_back(p.assign(path).append("\\").append(fileinfo.name));
				if (fileinfo.name[0] != '.')
					file_names.push_back(fileinfo.name);
				//cout << fileinfo.name << endl;
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

void getFiles(string path, vector<string>& files, vector<string>& file_names)
{
	//读取目录
	//long hFile = 0;//win32下
	intptr_t hFile = 0;//x64
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果不是目录
			if (!(fileinfo.attrib&_A_SUBDIR))
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
				file_names.push_back(fileinfo.name);
				//cout << fileinfo.name << endl;
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}

	//得到file_names
	string image_name;
	for (unsigned int i = 0; i < files.size(); ++i)
	{
		image_name = get_imgname(file_names[i]);
		file_names[i] = image_name;
	}
}



int main()
{
	string dbname = "SegTrackv2";
	vector<string> files_B;
	vector<string> file_names_B;
	string dataset_path;
	string save_path;
	float compactness = 6;
	vector<int> svnum;
	svnum.push_back(500);

#ifdef __linux__
	dataset_path = "../../Datasets/" + dbname + "/PNGImages";
	save_path = "../../Datasets/results/";
	char* curr_path = getcwd(NULL, 0);
	cout << curr_path << endl;
	getFileNames(dataset_path, file_names_B);
#endif
#ifdef WIN32
	//specify the input path of videos
	dataset_path = "D:\\workspace_vs17\\datasets\\" + dbname + "\\PNGImages";
	//specify the output path of supervoxel results, set output_label=true
	save_path = "D:\\workspace_vs17\\JFA-supervoxel-project\\anisotropic_supervoxel\\streamGPU-AS\\results\\";
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
			render.run(dataset_path, save_path, dbname, vdname, seed_num, compactness);
		}
	}
	system("pause");
	return 0;

}