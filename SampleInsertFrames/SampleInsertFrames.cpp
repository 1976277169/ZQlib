#include "ZQ_InsertFrames.h"
#include "windows.h"
#include "process.h"
#include <time.h>

using namespace ZQ;

class _insert_frames_arg
{
public:
	ZQ_InsertFramesOptions opt;
	bool done_flag;
};

void _insert_frames(void* arg)
{
	_insert_frames_arg* opt = (_insert_frames_arg*)arg;
	ZQ_InsertFrames::InsertFrames<float>((*opt).opt);
	opt->done_flag = true;
}

int main(const int argc, const char** argv)
{
	clock_t t1 = clock();
	ZQ_InsertFramesOptions opt;
	if (!opt.HandleArgs(argc - 1, argv + 1))
	{
		return 0;
	}

	if (opt.nCores > opt.frameNum - 1)
		opt.nCores = opt.frameNum - 1;

	if (opt.nCores > 1)
	{
		
		int each_frame_num = (opt.frameNum -1 + opt.nCores - 1) / opt.nCores;
		_insert_frames_arg* tmp_args = new _insert_frames_arg[opt.nCores];
		for (int i = 0; i < opt.nCores; i++)
		{
			tmp_args[i].opt = opt;
			tmp_args[i].opt.baseId = opt.baseId + each_frame_num * i;
			if (i == opt.nCores - 1)
			{
				tmp_args[i].opt.frameNum = opt.frameNum - each_frame_num*i;
			}
			else
			{
				tmp_args[i].opt.frameNum = each_frame_num + 1;
			}
			tmp_args[i].opt.outBaseId = opt.outBaseId + each_frame_num*i*opt.speedUp;
			tmp_args[i].done_flag = false;
		}

		for (int i = 0; i < opt.nCores - 1;i++)
			_beginthread(_insert_frames, 0, &tmp_args[i]);
		_insert_frames(&tmp_args[opt.nCores - 1]);

		bool all_done;
		do{
			Sleep(10);
			all_done = true;
			for (int i = 0; i < opt.nCores; i++)
			{
				if (!tmp_args[i].done_flag)
				{
					all_done = false;
					break;
				}
			}
		} while (!all_done);
		delete[]tmp_args;
	}
	else
	{
		ZQ_InsertFrames::InsertFrames<float>(opt);
	}
	
	clock_t t2 = clock();
	printf("cost time:%f seconds\n", 0.001*(t2 - t1));
	return 0;
}


int main2()
{
	/************* ��ȡͼ�� ****************/
	//ZQ_DImage<float> ���Զ���ͼ���ʽ������˳���opencv���ƣ�����ͼ��������0.0-1.0
	//�ҵ��ļ�ZQ_ImageIO.h ��loadImage���������Բ鿴����δ�IplImage ת��ZQ_DImage<float>
	ZQ_DImage<float> im1, im2;
	ZQ_DImage<float> bgr1, bgr2, out_im;
	ZQ_ImageIO::loadImage(im1, "1.jpg",0);	//��ȡ�Ҷ�ͼ�����ڹ���
	ZQ_ImageIO::loadImage(im2, "2.jpg",0);	
	ZQ_ImageIO::loadImage(bgr1, "1.jpg", 1);	//��ȡ��ɫͼ�����ڲ�֡
	ZQ_ImageIO::loadImage(bgr2, "2.jpg", 1);
	int width = im1.width();
	int height = im1.height();
	float weight1 = 0.5;	//��֡ʱ��һ֡ͼ���Ȩ�أ�ȡֵ��Χ(0.0, 1.0)
	float scale = 0.25;	//���ƹ���ʱ������С�˽��й��ƣ�����ȡ0.25����0.5
	float scale_back = 1.0 / scale;
	im1.imresize(scale);
	im2.imresize(scale);

	/************** L2 ʹ��ʾ��  ************/
	/* L2 �����̶��Ĳ�����Ӧ�Լ�ǿ��������Ҳ�ǳ�С��ʹ��Ĭ�ϲ�������֡ͼ������7֡�����߳�Լ6�루������д�ļ���
	*/
	ZQ_DImage<float> fw_u_L2, fw_v_L2, warpIm_L2;
	ZQ_OpticalFlowOptions opt_L2;	//Ĭ�ϲ���������Ѿ��������һ�㲻Ҫ�Ķ������Ǿ����ر�ḻ
	ZQ_OpticalFlow::Coarse2Fine_HS_L2(fw_u_L2, fw_v_L2, warpIm_L2, im1, im2, opt_L2);
	fw_u_L2.imresize(width, height);	//ע��Ŵ��ԭ���߶�
	fw_v_L2.imresize(width, height);
	fw_u_L2.Multiplywith(scale_back);	//ע�������ֵҲҪ�Ŵ�
	fw_v_L2.Multiplywith(scale_back);
	if (!ZQ_InsertFrames::InsertOneFrameWithFlow_Simple(bgr1, bgr2, fw_u_L2, fw_v_L2, weight1, out_im))
	{
		printf("failed to run InsertOneFrameWithFlow_Simple!\n");
		return 0;
	}
	ZQ_ImageIO::saveImage(out_im, "out_L2.jpg");

	/*************** L1 ʹ��ʾ�� ***************/
	/* L1 �����������࣬������Ӧ�Բ壬�������ǳ���ʹ��Ĭ�ϲ�������֡ͼ������7֡�����߳�Լ196�루������д�ļ���
	*/
	ZQ_DImage<float> fw_u_L1, fw_v_L1, bw_u_L1, bw_v_L1, warpIm_L1;
	ZQ_OpticalFlowOptions opt_L1;
	ZQ_OpticalFlowOptions::GetDefaultOptions_HS_L1(opt_L1);//Ĭ�ϲ���������Ѿ��������һ�㲻Ҫ�Ķ������Ǿ����ر�ḻ
	ZQ_OpticalFlow::Coarse2Fine_HS_L1(fw_u_L1, fw_v_L1, warpIm_L1, im1, im2, opt_L1);//����im1 ��im2�Ĺ���
	ZQ_OpticalFlow::Coarse2Fine_HS_L1(bw_u_L1, bw_v_L1, warpIm_L1, im2, im1, opt_L1);//����im2 ��im1�Ĺ���
	fw_u_L1.imresize(width, height);	//ע��Ŵ��ԭ���߶�
	fw_v_L1.imresize(width, height);
	fw_u_L1.Multiplywith(scale_back);	//ע�������ֵҲҪ�Ŵ�
	fw_v_L1.Multiplywith(scale_back);
	bw_u_L1.imresize(width, height);	//ע��Ŵ��ԭ���߶�
	bw_v_L1.imresize(width, height);
	bw_u_L1.Multiplywith(scale_back);	//ע�������ֵҲҪ�Ŵ�
	bw_v_L1.Multiplywith(scale_back);
	if (!ZQ_InsertFrames::InsertOneFrameWithFlow_Complex(bgr1, bgr2, fw_u_L1, fw_v_L1, bw_u_L1, bw_v_L1, weight1, out_im))//ע����Ĭ�ϲ������Լ�������������أ�һ�������Ҫ�Ķ�����
	{
		printf("failed to run InsertOneFrameWithFlow_Complex!\n");
		return 0;
	}
		
	ZQ_ImageIO::saveImage(out_im, "out_L1.jpg");
	
	return 0;
}