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
	unsigned char DataSubType; // 6 19	1	数据分类型	17值为0x01时 - 0x01~0x07 AM / FM / PM / USB / LSB / ISB / CW
	unsigned char Reserved[13]; // 7 20 - 32	13	补零
};

struct DataWB_FFT
{
	static constexpr auto LENGTH = 10240;
	CommonParams params;
	unsigned short Data[LENGTH];
	unsigned char Reserved[48];
};

struct DDC
{
	short I;
	short Q;
};

struct DataNB_DDC
{
	static constexpr auto LENGTH = 256;
	CommonParams params;
	DDC DDCData[LENGTH];
};

#pragma pack()

#endif
