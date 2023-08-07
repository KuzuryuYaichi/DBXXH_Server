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
        //	  1.1   �豸�Լ�ָ��
        //    �豸�Լ�ָ���ָ�������е�1��2�ֽ���Ч�����±���ʾ��
        //    �� 4�����豸�Լ�
        //    ���	����	ָ������	˵��
        //    ��1~2�ֽ�	��3~10�ֽ�
        //    0x01	�豸�Լ�	0xAAAA	ȫ0
        //    �豸�Լ�ָ�����ʮ������������
        //    FA A5 FB B5 FC C5 FF 01 AA AA 00 00 00 00 00 00 00 00 F5 5F
            struct SelfCheck
            {
                unsigned short Ctrl = 0xAAAA;
                char Reserved[8];
            };
        //    1.2	����������
        //    �������£�
        //    �� 8 ��������ͨ������
        //    ָ������	ָ������
        //    ��1�ֽ�	��2�ֽ�	��1~2�ֽ�	0x0001����1ͨ��
        //    0xFE	0x21	��3~4�ֽ�[15:14]��������������Ϊ��00��
        //    [13]--��0������С���壬��1��������С����
        //    [12]--��0���������2ms����1�� �������1ms
        //    [11:0]��������ֵ(���ֵ4095��Ĭ����СֵΪ40)
        //    Ĭ��x��3080��
        //    ��5~6�ֽ�	����С��������pw * 204.8(us)(����ֵ����ȡ��)
        //    ��Ĭ��0.27us ��������56��
        //    ��7~8�ֽ�	���÷������ƣ�Ĭ��x��0000����
            struct PulseDemodeParam
            {
                short Channel;
                unsigned short Ctrl;
                unsigned short Gate;
                unsigned short PulseWidthDepress;
                unsigned short PulseRangeDepress;
                char Reserved[2];
            };
        //    1.3	FFT���ò���
        //    ƽ�����������FFT����������������
        //    �� 9 ����FFT���ò���
        //    ָ������	ָ������	Ĭ��ֵ
        //    ��1�ֽ�	��2�ֽ�	��1�ֽ�	Ƶ��ƽ������4~10
        //    ��Ӧ��x��04�� ~x��0A��	x��04��
        //    0xFE	0x22	��2�ֽ�	���FFT������1024~16384
        //    ��Ӧ��x��0A��~x��0E��	x��0A��
        //    ��3�ֽ�	����������
        //    x��01��-- - blackman  x��02��-- - flagtop
        //    x��03��-- - hanning  x��04��-- - hamming
        //    x��05��-- - kaiser	x��02��
            struct FFT_Param
            {
                char DataPoints;
                unsigned short Smooth;
                char SmoothLog;
                char WinType;
                char Reserved[5];
            } FFT_Param;
        //    1.4	���DDC��������
        //    DDS�˲������ã�Ƶ�׹۲��ȡ����
        //    �� 10 ����DDC���ò���
        //    ָ������	ָ������	Ĭ��ֵ
        //    ��1�ֽ�	��2�ֽ�	��1~4�ֽ�	DDS�˲������ã�ȫƵ�׹۲���DDS����Ƶ��15M���ֲ��۲�������Ƶ������ɵ�	x��01400000��
        //    0xFE	0x23
        //    ��5�ֽ�	Ƶ�׹۲��ȡ������2~64
        //    ��Ӧ��x��02�� ~x��06��	x��02��

        //    1.5	խ���������
        //    խ������ddc����Ƶ�ʣ�ʶ���ź�ģʽ��Ƶ���Ʋ���
        //    ָ������	ָ������
        //    ��1�ֽ�	��2�ֽ�	��1~2�ֽ�
        //    0xF1	0x24	��3�ֽ�	����ѡ��ӳ��
        //    0x00 0.1KHz
        //    0x09 500KHz
        //    0x0A 1MHz
        //    0x0B 2MHz
        //    0x0C 5MHz
        //    0x0D 10MHz
        //    0x0E 20MHz
        //    0x0F 40MHz
        //    ��4�ֽ�	�����ʽ 0x01~0x08 :AM / FM / PM / USB / LSB / ISB / CW / PULSE
        //    ��5~8�ֽ�	ddc����Ƶ�� = fc / fs * 2 ^ 32(ע��fcΪĿ��Ƶ�ʣ�fsΪ204.8MHz�Ĳ�����)

        //    1.6	��Ƶģ����Ʋ���
        //    Ĭ���ϵ�״̬��
        //    ��ƵƵ�㣺500MHz
        //    ˥��ֵ��0dB(����Ƶ�������)
        //    ����ģʽ������
        //    ��Ƶ����72MHz
        //    ɨƵģʽ : ��Ƶ
        //    ָ������	ָ������
        //    ��1�ֽ�	��2�ֽ�	��1�ֽ�	�������� 0x01~04
        //    0xF1	0x25	��2~6�ֽ�	��������Ϊ��ʼƵ��0x01 : Ƶ���� / Hz
        //    ��2~6�ֽ�	��������Ϊ��ֹƵ��0x02 : Ƶ���� / Hz
        //    ��2�ֽ�	��������Ϊ����ģʽ0x03 : ���� / ����ţ�0x00 / 0x01
        //    ��3�ֽ�	��������Ϊ����ģʽ0x03 : ��Ƶ˥��ֵ0~31 / dB
        //    ��2�ֽ�	��������ΪɨƵģʽ0x04 : ��ʼ / ֹͣɨƵ��0x00 / 0x01
        //    ��3�ֽ�	��������ΪɨƵģʽ0x04 : ����72 / 40 / 20��0x00 / 01 / 02

        //    ������ƵģʽƵ��200MHz ��������ģʽ ��Ƶ˥��5 ����72ʱ�·�ָ��
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