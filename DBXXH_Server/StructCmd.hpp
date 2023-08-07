#ifndef _STRUCT_CMD
#define _STRUCT_CMD

#include <filesystem>
#include <fstream>
#include <iostream>
#include <cmath>
#include "../XDMA_PCIE/dllexport.h"
#include "global.h"

namespace DBXXH
{
#pragma pack(1)

    struct StructCmdWB
    {
        unsigned char Head[6] = { (unsigned char)0xFA, (unsigned char)0xA5, (unsigned char)0xFB, (unsigned char)0xB5, (unsigned char)0xFC, (unsigned char)0xC5 };
        unsigned short Type;
        union Context
        {
        //	  1.1   设备自检指令
        //    设备自检指令的指令内容中第1～2字节有效，如下表所示：
        //    表 4设置设备自检
        //    类别	定义	指令内容	说明
        //    第1~2字节	第3~10字节
        //    0x01	设备自检	0xAAAA	全0
        //    设备自检指令范例（十六进制数）：
        //    FA A5 FB B5 FC C5 FF 01 AA AA 00 00 00 00 00 00 00 00 F5 5F
            struct SelfCheck
            {
                unsigned short Ctrl = 0xAAAA;
                char Reserved[8];
            };
        //    1.2	脉冲解调参数
        //    定义如下：
        //    表 8 设置脉冲通道参数
        //    指令类型	指令内容
        //    第1字节	第2字节	第1~2字节	0x0001：第1通道
        //    0xFE	0x21	第3~4字节[15:14]保留，必须设置为”00”
        //    [13]--‘0’容许小脉冲，‘1’不容许小脉冲
        //    [12]--‘0’最大脉宽2ms，‘1’ 最大脉宽1ms
        //    [11:0]设置门限值(最大值4095，默认最小值为40)
        //    默认x“3080”
        //    第5~6字节	设置小脉宽抑制pw * 204.8(us)(最终值向上取整)
        //    （默认0.27us 即下设置56）
        //    第7~8字节	设置幅度抑制（默认x”0000”）
            struct PulseDemodeParam
            {
                short Channel;
                unsigned short Ctrl;
                unsigned short Gate;
                unsigned short PulseWidthDepress;
                unsigned short PulseRangeDepress;
                char Reserved[2];
            };
        //    1.3	FFT配置参数
        //    平滑次数，宽带FFT点数，窗函数类型
        //    表 9 设置FFT配置参数
        //    指令类型	指令内容	默认值
        //    第1字节	第2字节	第1字节	频谱平滑次数4~10
        //    对应于x”04” ~x”0A”	x”04”
        //    0xFE	0x22	第2字节	宽带FFT点数：1024~16384
        //    对应于x”0A”~x”0E”	x”0A”
        //    第3字节	窗函数类型
        //    x”01”-- - blackman  x”02”-- - flagtop
        //    x”03”-- - hanning  x”04”-- - hamming
        //    x”05”-- - kaiser	x”02”
            struct FFT_Param
            {
                char DataPoints;
                unsigned short Smooth;
                char SmoothLog;
                char WinType;
                char Reserved[5];
            } FFT_Param;
        //    1.4	宽带DDC参数配置
        //    DDS核参数配置，频谱观测抽取倍数
        //    表 10 设置DDC配置参数
        //    指令类型	指令内容	默认值
        //    第1字节	第2字节	第1~4字节	DDS核参数配置，全频谱观测下DDS中心频点15M，局部观测下中心频点任意可调	x”01400000”
        //    0xFE	0x23
        //    第5字节	频谱观测抽取倍数：2~64
        //    对应于x”02” ~x”06”	x”02”

        //    1.5	窄带解调参数
        //    窄带带宽，ddc中心频率，识别信号模式射频控制参数
        //    指令类型	指令内容
        //    第1字节	第2字节	第1~2字节
        //    0xF1	0x24	第3字节	带宽选择映射
        //    0x00 0.1KHz
        //    0x09 500KHz
        //    0x0A 1MHz
        //    0x0B 2MHz
        //    0x0C 5MHz
        //    0x0D 10MHz
        //    0x0E 20MHz
        //    0x0F 40MHz
        //    第4字节	解调方式 0x01~0x08 :AM / FM / PM / USB / LSB / ISB / CW / PULSE
        //    第5~8字节	ddc中心频率 = fc / fs * 2 ^ 32(注：fc为目标频率，fs为204.8MHz的采样率)

        //    1.6	射频模块控制参数
        //    默认上电状态：
        //    射频频点：500MHz
        //    衰减值：0dB(即射频增益最大)
        //    工作模式：常规
        //    中频带宽：72MHz
        //    扫频模式 : 定频
        //    指令类型	指令内容
        //    第1字节	第2字节	第1字节	数据类型 0x01~04
        //    0xF1	0x25	第2~6字节	数据类型为起始频率0x01 : 频率字 / Hz
        //    第2~6字节	数据类型为截止频率0x02 : 频率字 / Hz
        //    第2字节	数据类型为增益模式0x03 : 常规 / 低噪放：0x00 / 0x01
        //    第3字节	数据类型为增益模式0x03 : 射频衰减值0~31 / dB
        //    第2字节	数据类型为扫频模式0x04 : 开始 / 停止扫频：0x00 / 0x01
        //    第3字节	数据类型为扫频模式0x04 : 带宽72 / 40 / 20：0x00 / 01 / 02

        //    例：定频模式频率200MHz 常规增益模式 射频衰减5 带宽72时下发指令
        //    FA A5 FB B5 FC C5 F1 24 01 0B EB C2 00 00 00 00 00 00 F5 5F
        //    FA A5 FB B5 FC C5 F1 24 02 0B EB C2 00 00 00 00 00 00 F5 5F
        //    FA A5 FB B5 FC C5 F1 24 03 00 05 00 00 00 00 00 00 00 F5 5F
        //    FA A5 FB B5 FC C5 F1 24 04 01 00 00 00 00 00 00 00 00 F5 5F
        } Context;
        unsigned short Tail = 0x5FF5;

        StructCmdWB() {}

        void SendCXCmd()
        {
            WriteStreamCmd((char*)this, sizeof(StructCmdWB), 0);
        }
    };

    struct StructCmdZC
    {
        unsigned char CmdType = 0;
        union
        {
            struct CMD_NB {
                unsigned char Channel;
                unsigned short CIC = 200;
                unsigned int DDS;
            } CmdNB;
            struct CMD_RF {
                unsigned char RfType;
                unsigned int RfData;
                unsigned char Reserved[2];
            } CmdRF;
        };

        StructCmdZC() {}

        void SendZCCmd()
        {
            WriteStreamCmd((char*)this, sizeof(StructCmdZC), 1);
        }
    };

#pragma pack()

    struct Order
    {
        char* order = nullptr;
        Order() {}
        Order(const Order&) = default;
        Order& operator=(const Order&) = default;

        Order(size_t len) : order(new char[len]) {}
        ~Order()
        {
            if (order != nullptr)
                delete[] order;
        }
    };
}

#endif