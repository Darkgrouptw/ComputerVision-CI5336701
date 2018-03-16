#include <iostream>
#include <algorithm>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

//////////////////////////////////////////////////////////////////////////
// 顯示檔案格式
//////////////////////////////////////////////////////////////////////////
void	PrintDescription()
{
	cout << "請按照以下方式處理" << endl;
	cout << "<.exe> <parmeter.txt>" << endl;
	cout << "請確定 001.bmp ~ 0.08.bmp 和 camera parameter.txt 在同一個目錄下" << endl;
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
		doList[i] = TransformPath + "00" + to_string(i) + ".bmp";
	doList[8] = paramsFile;
	return doList;
}


int main(int argc, char ** argv)
{
	if (argc != 2)
	{
		PrintDescription();
		return -1;
	}
	string FileName = string(argv[1]);
	string *doList = GetDoList(FileName);
	//imread()

	for (int i = 0; i < 9; i++)
		cout << doList[i] << endl;
	delete(&doList);
	return 0;
}