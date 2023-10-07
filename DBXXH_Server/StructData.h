#ifndef _STRUCT_DATA_H
#define _STRUCT_DATA_H

#include <memory>

#pragma pack(1)

struct CommonParams
{
	unsigned int Head;// 1 1~4	4	ͬ��ͷ	1�ֽ�Ϊ��λ��4�ֽ�Ϊ��λ��ͬ��ͷ�̶�Ϊ0xABCD1234��С�ˣ�
	unsigned int PackNum; // 2 5~8	4   ����ˮ��	0x00000000~0xFFFFFFFF
	unsigned char Time[8]; // 3 9~16	8	TOAʱ�� ������ʱ���룩TOA[64:0]	TOA[64:58]���� 0~99 TOA[57:49]����1~356 TOA[48:44]��ʱ - 24Сʱ�� TOA[43:38]���� - 0~59 TOA[37:32]���� - 0~59
	unsigned char DataType; // 4 17	1	��������	0x01��խ��ģ���������0x02��խ��DDC IQ��0x03�����FFT���ݣ�0x04������������
	unsigned char ChNum;// 5 18	1	ͨ����	17ֵΪ0x01 / 02ʱ - x01~0x08Ϊͨ�����
	unsigned char Demod; // 6 19	1	���ݷ�����	17ֵΪ0x01ʱ - 0x01~0x07 AM / FM / PM / USB / LSB / ISB / CW
	union
	{
		struct WBParams_
		{
			unsigned char Resolution; //1	���FFT�ֱ���	0x0A~0x0EΪ���FFT�ֱ������
			unsigned char Smooth; //1	���FFTƽ������	0x04��0x08��0x10��0x20��0x40��Ӧ��4��8��16��32��64��ƽ��
			unsigned char SimBand; //1	�Ŵ���	Ĭ��0x02    0x02 - 0x40
			unsigned int DDS; //1	DDS����Ƶ��	Ĭ��15M ��0x01400000
			unsigned char Reserved[6]; // 7 20 - 32	13	����
		} WBParams;
		struct NBParams_
		{
			unsigned short CIC;
			unsigned int DDS;
			union
			{
				struct AM_Params_
				{
					unsigned char Reserved[3]; // 3 30 - 32	3	����
					unsigned short AM_DC;
					unsigned short AM_DataMax;
					
				} AM_Params;
				struct Reserved_
				{
					unsigned char Reserved[7]; // 7 26 - 32	7	����
				} Reserved;
			};
		} NBParams;
	};
};

typedef unsigned short FFT;

struct DataWB_Data
{
	static constexpr auto LENGTH = 10240;
	CommonParams Params;
	FFT Data[LENGTH];
	unsigned char Reserved[48];
};

struct DDC
{
	short I;
	short Q;
};

struct Pulse
{
	unsigned int PulseWidth; // ��2�ֽ�	����ĸ�8λ��23��16�� ֵת����ʮ���� / 96 ��λ��us ��3��4�ֽ�	����ĵ�16λ��15��0��
	unsigned short PulseAmpl; // ��5��6�ֽ�	����	ֱ����ʾ
	unsigned short DDS; // ��7��8�ֽ� Ƶ�ʲ���	ֵת����ʮ���� * 96 / 65536��λ��MHz
	unsigned int Time; // ��9��12�ֽ� TOA[63:32]
	unsigned int Nanoseconds; // ��13��16�ֽ� ���ڼ�����
};

struct DataNB_Data
{
	static constexpr auto DDC_LENGTH = 256, PULSE_LENGTH = 16;
	CommonParams Params;
	union
	{
		DDC DDCData[DDC_LENGTH];
		Pulse PulseData[PULSE_LENGTH];
	};
};

#pragma pack()

#endif
