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

#pragma region �ܼƶ��X
Mat *IntrinsicMat;														// �����Ѽ�
Mat *ExtrinsicMat[8];													// �~���Ѽ�

Mat ImgMat[8];															// �Ӥ�
bool *_IsModel;															// �@���� Model (���F�[�t��)
bool ***IsModel;														// �O�_�O�ҫ����P�_
#pragma endregion

//////////////////////////////////////////////////////////////////////////
// ����ɮ׮榡
//////////////////////////////////////////////////////////////////////////
void	PrintDescription()
{
	cout << "�Ы��ӥH�U�覡�B�z" << endl;
	cout << "<.exe> <parmeter.txt> <output.XYZ>" << endl;
	cout << "�нT�w 001.bmp ~ 0.08.bmp �M camera parameter.txt �b�P�@�ӥؿ��U" << endl;
	cout << "���ͪ�XYZ���ؿ��|�H exe ���D" << endl;
}

//////////////////////////////////////////////////////////////////////////
// �ھڦr���Ҧ��� bmp & txt �[�i�h
// 0 ~7 => bmp
// 8	=> txt 
//////////////////////////////////////////////////////////////////////////
string*	GetDoList(string paramsFile)
{
	// �r��B�z
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
// �}�Ҩî����Ѽ�
//////////////////////////////////////////////////////////////////////////
void OpenFileParams(string FileName)
{
	fstream fp;
	fp.open(FileName.c_str(), ios::in);
	if (!fp)
	{
		cout << "�ɮ׶}�ҿ��~!!" << endl;
		exit(-1);
	}

	// ��l��
	IntrinsicMat = new Mat(3, 3, CV_32FC1);
	for (int i = 0; i < 8; i++)
		ExtrinsicMat[i] = new Mat(3, 4, CV_32FC1);

	// ������
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
// �}�ҹϤ�
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
	#pragma region ��l�� bool
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
	#pragma region  �]�Ϥ�
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
						// �]�w�@�ɼ�
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
	#pragma region ���յ��G
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
// �s��
//////////////////////////////////////////////////////////////////////////
void SaveToXYZ(string OutputFileName)
{
	#pragma region �}�ɮ�
	fstream fp;
	fp.open(OutputFileName, ios::out);
	#pragma endregion
	#pragma region �s��
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

	#pragma region �P���J�O�_���~
	if (argc != 3)
	{
		PrintDescription();
		return -1;
	}
	#pragma endregion
	#pragma region ���� doList & �ö}�ҰѼ��� & �Ϥ�
	string FileName = string(argv[1]);
	string OutputFileName = string(argv[2]);
	string *doList = GetDoList(FileName);

	// �Ѽ�
	OpenFileParams(doList[8]);

	// ��
	OpenImage(doList);
	#pragma endregion
	#pragma region �]��������� & �s��
	RunThroughAllData();

	SaveToXYZ(OutputFileName);
	#pragma endregion
	#pragma region �M��
	delete[] doList;
	delete IntrinsicMat;
	delete[] _IsModel;
	#pragma endregion
	//system("PAUSE");
	endTimer = clock();
	cout << (float)(endTimer - startTimer) / CLOCKS_PER_SEC << " sec" << endl;
	return 0;
}