#ifndef _STRUCT_DATA_H
#define _STRUCT_DATA_H

#include <memory>

#pragma pack(1)

struct CommonParams
{
	unsigned int Head;// 1 1~4	4	同步头	1字节为低位，4字节为高位，同步头固定为0xABCD1234（小端）
	unsigned int PackNum; // 2 5~8	4   包流水号	0x00000000~0xFFFFFFFF
	unsigned char Time[8]; // 3 9~16	8	TOA时标 （年日时分秒）TOA[64:0]	TOA[64:58]：年 0~99 TOA[57:49]：日1~356 TOA[48:44]：时 - 24小时制 TOA[43:38]：分 - 0~59 TOA[37:32]：秒 - 0~59
	unsigned char DataType; // 4 17	1	数据类型	0x01：窄带模拟解调结果；0x02：窄带DDC IQ；0x03：宽带FFT数据；0x04：脉冲监测结果；
	unsigned char ChNum;// 5 18	1	通道号	17值为0x01 / 02时 - x01~0x08为通道序号
	unsigned char Demod; // 6 19	1	数据分类型	17值为0x01时 - 0x01~0x07 AM / FM / PM / USB / LSB / ISB / CW
	union
	{
		struct WBParams_
		{
			unsigned char Resolution; //1	宽带FFT分辨率	0x0A~0x0E为宽带FFT分辨率序号
			unsigned char Smooth; //1	宽带FFT平滑次数	0x04、0x08、0x10、0x20、0x40对应于4、8、16、32、64次平滑
			unsigned char SimBand; //1	放大倍数	默认0x02    0x02 - 0x40
			unsigned int DDS; //1	DDS中心频点	默认15M 即0x01400000
			unsigned char Reserved[6]; // 7 20 - 32	13	补零
		} WBParams;
		struct NBParams_
		{
			unsigned short CIC;
			unsigned int DDS;
			union
			{
				struct AM_Params_
				{
					unsigned char Reserved[3]; // 3 30 - 32	3	补零
					unsigned short AM_DC;
					unsigned short AM_DataMax;
					
				} AM_Params;
				struct Reserved_
				{
					unsigned char Reserved[7]; // 7 26 - 32	7	补零
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
	unsigned int PulseWidth; // 第2字节	脉宽的高8位（23～16） 值转换成十进制 / 96 单位：us 第3～4字节	脉宽的低16位（15～0）
	unsigned short PulseAmpl; // 第5～6字节	脉幅	直接显示
	unsigned short DDS; // 第7～8字节 频率参数	值转换成十进制 * 96 / 65536单位：MHz
	unsigned int Time; // 第9～12字节 TOA[63:32]
	unsigned int Nanoseconds; // 第13～16字节 秒内计数器
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
