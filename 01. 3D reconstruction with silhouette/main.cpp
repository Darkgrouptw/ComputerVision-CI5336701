#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <time.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

#pragma region 變數集合
Mat *IntrinsicMat;														// 內部參數
Mat *ExtrinsicMat[8];													// 外部參數

Mat ImgMat[8];															// 照片
bool *_IsModel;															// 一維的 Model (為了加速用)
bool ***IsModel;														// 是否是模型的判斷
#pragma endregion

//////////////////////////////////////////////////////////////////////////
// 顯示檔案格式
//////////////////////////////////////////////////////////////////////////
void	PrintDescription()
{
	cout << "請按照以下方式處理" << endl;
	cout << "<.exe> <parmeter.txt> <output.XYZ>" << endl;
	cout << "請確定 001.bmp ~ 0.08.bmp 和 camera parameter.txt 在同一個目錄下" << endl;
	cout << "產生的XYZ的目錄會以 exe 為主" << endl;
}

//////////////////////////////////////////////////////////////////////////
// 根據字串把所有的 bmp & txt 加進去
// 0 ~7 => bmp
// 8	=> txt 
//////////////////////////////////////////////////////////////////////////
string*	GetDoList(string paramsFile)
{
	// 字串處理
	string TransformPath = paramsFile;
	std::replace(TransformPath.begin(), TransformPath.end(), '/', '\\');
	int lastIndex = TransformPath.find_last_of('\\');
	TransformPath = TransformPath.substr(0, lastIndex) + "\\";

	string* doList = new string[9];
	for (int i = 0; i < 8; i++)
		doList[i] = TransformPath + "00" + to_string(i + 1) + ".bmp";
	doList[8] = paramsFile;
	return doList;
}

//////////////////////////////////////////////////////////////////////////
// 開啟並拿取參數
//////////////////////////////////////////////////////////////////////////
void OpenFileParams(string FileName)
{
	fstream fp;
	fp.open(FileName.c_str(), ios::in);
	if (!fp)
	{
		cout << "檔案開啟錯誤!!" << endl;
		exit(-1);
	}

	// 初始化
	IntrinsicMat = new Mat(3, 3, CV_32FC1);
	for (int i = 0; i < 8; i++)
		ExtrinsicMat[i] = new Mat(3, 4, CV_32FC1);

	// 抓取資料
	string line;
	bool IsIntrinsic = true;
	int EntrinsicIndex = 0;
	while (std::getline(fp, line))
	{
		if (line[0] == '#')
		{
			for (int i =0; i <3; i++)
			{
				std::getline(fp, line);
				istringstream ss(line);
				float a, b, c, d;
				if (IsIntrinsic)
				{
					ss >> a >> b >> c;
					IntrinsicMat->at<float>(i, 0) = a;
					IntrinsicMat->at<float>(i, 1) = b;
					IntrinsicMat->at<float>(i, 2) = c;
				}
				else
				{
					ss >> a >> b >> c >> d;
					ExtrinsicMat[EntrinsicIndex]->at<float>(i, 0) = a;
					ExtrinsicMat[EntrinsicIndex]->at<float>(i, 1) = b;
					ExtrinsicMat[EntrinsicIndex]->at<float>(i, 2) = c;
					ExtrinsicMat[EntrinsicIndex]->at<float>(i, 3) = d;
				}
			}
			if (!IsIntrinsic)
				EntrinsicIndex++;
			IsIntrinsic = false;
		}
	}
	fp.close();
	//cout << *(ExtrinsicMat[0]) << endl;
}

//////////////////////////////////////////////////////////////////////////
// 開啟圖片
//////////////////////////////////////////////////////////////////////////
void OpenImage(string *doList)
{
	for (int i = 0; i < 8; i++)
		ImgMat[i] = imread(doList[i].c_str(), IMREAD_GRAYSCALE);
}

//////////////////////////////////////////////////////////////////////////
// Run through all data
//////////////////////////////////////////////////////////////////////////
void RunThroughAllData()
{
	#pragma region 初始化 bool
	int tableSize = 100 * 100 * 100;
	_IsModel = new bool[tableSize];
	memset(_IsModel, 1, sizeof(bool) * tableSize);
	IsModel = new bool**[100];
	for (int i = 0; i < 100; i++)
	{
		IsModel[i] = new bool*[100];
		for (int j = 0; j < 100; j++)
			IsModel[i][j] = &_IsModel[i * 100 * 100 + j * 100];
	}
	#pragma endregion
	#pragma region  跑圖片
	for (int i = 0; i < 8; i++)
		#pragma omp parallel for
		for (int z = -10; z < 90; z++)
			for(int y = -50; y < 50; y++)
				for (int x = -50; x < 50; x++)
				{
					int XIndex = x + 50;
					int YIndex = y + 50;
					int ZIndex = z + 10;
					
					if (IsModel[XIndex][YIndex][ZIndex])
					{
						// 設定世界標
						Mat World(4, 1, CV_32FC1);
						World.at<float>(0, 0) = x;
						World.at<float>(1, 0) = y;
						World.at<float>(2, 0) = z;
						World.at<float>(3, 0) = 1;


						Mat pixel = *(IntrinsicMat) * *(ExtrinsicMat[i]) * World;
						float farZ = pixel.at<float>(2, 0);
						int pixelX = int(pixel.at<float>(0, 0) / farZ);
						int pixelY = int(pixel.at<float>(1, 0) / farZ);

						if (pixelY < 0 || pixelY >= 600 || pixelX < 0 || pixelX >= 800)
							IsModel[XIndex][YIndex][ZIndex] = false;
						else if (ImgMat[i].at<uchar>(pixelY, pixelX) != 255)
							IsModel[XIndex][YIndex][ZIndex] = false;
					}
				}
	#pragma endregion
	#pragma region 測試結果
	/*for (int i = 0; i < 100; i ++)
		for (int j = 0; j < 100; j++)
			for (int k = 0; k < 100; k++)
			{
				if (IsModel[i][j][k])
					cout << i << " " << j << " " << k << endl;
			}*/
	#pragma endregion
}

//////////////////////////////////////////////////////////////////////////
// 存檔
//////////////////////////////////////////////////////////////////////////
void SaveToXYZ(string OutputFileName)
{
	#pragma region 開檔案
	fstream fp;
	fp.open(OutputFileName, ios::out);
	#pragma endregion
	#pragma region 存檔
	for (int i =0; i < 100; i++)
		for (int j = 0; j < 100; j++)
			for (int k = 0; k < 100; k++)
			{
				if (IsModel[i][j][k])
					fp << i << " " << j << " " << k << endl;
			}
	fp.close();
	#pragma endregion
}

int main(int argc, char ** argv)
{
	clock_t startTimer, endTimer;
	startTimer = clock();

	#pragma region 判對輸入是否錯誤
	if (argc != 3)
	{
		PrintDescription();
		return -1;
	}
	#pragma endregion
	#pragma region 產生 doList & 並開啟參數檔 & 圖片
	string FileName = string(argv[1]);
	string OutputFileName = string(argv[2]);
	string *doList = GetDoList(FileName);

	// 參數
	OpenFileParams(doList[8]);

	// 圖
	OpenImage(doList);
	#pragma endregion
	#pragma region 跑全部的資料 & 存檔
	RunThroughAllData();

	SaveToXYZ(OutputFileName);
	#pragma endregion
	#pragma region 清空
	delete[] doList;
	delete IntrinsicMat;
	delete[] _IsModel;
	#pragma endregion
	//system("PAUSE");
	endTimer = clock();
	cout << (float)(endTimer - startTimer) / CLOCKS_PER_SEC << " sec" << endl;
	return 0;
}