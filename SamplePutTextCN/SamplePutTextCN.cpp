#include "ZQ_PutTextCN.h"
using namespace ZQ;
using namespace cv;
int main()
{
	Mat img = imread("input2.png");

	ZQ_PutTextCN::PutTextCN(img, "Arial���廻...\n����ʾ!", Point(50, 50), Scalar(0, 0, 255), 30, "Arial");
	ZQ_PutTextCN::PutTextCN(img, "΢���ź����廻...\n�У�б�壬�»��ߣ���ʾ!", Point(50, 100), Scalar(0, 255, 0), 30, "΢���ź�", true, true);
	ZQ_PutTextCN::PutTextCN(img, "�������廻...\n�У�б�壬�»��ߣ���ʾ!", Point(50, 200), Scalar(128, 255, 0), 30, "����", true, true);

	imshow("test", img);

	waitKey();

	return 0;
}