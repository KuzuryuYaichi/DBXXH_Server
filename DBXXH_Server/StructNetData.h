#ifndef _STRUCT_NET_DATA_H
#define _STRUCT_NET_DATA_H

#include <memory>
#include <mutex>
#include <cmath>
#include "StructData.h"
#include "global.h"

namespace DBXXH
{
#pragma pack(1)

    struct DataHead
    {
        unsigned int Head = 0xF99FEFFE;
        unsigned char Version = 0x30;
        unsigned char IsEnd = 0;
        unsigned int PackLen;
        unsigned short PackNum = 0;
        unsigned short PackType;
        unsigned char DeviceID[14];
        unsigned char TaiShiMode = 0xFF;
        unsigned short UnUsed = 0xFFFF;
        unsigned char Security = 4;

        DataHead(size_t PackLen, unsigned short PackType, unsigned char* DeviceID) : PackLen(PackLen), PackType(PackType)
        {
            std::memcpy(this->DeviceID, DeviceID, sizeof(this->DeviceID));
        }

        DataHead(size_t PackLen, unsigned short PackType, unsigned char* DeviceID, unsigned short PackNum) : PackLen(PackLen), PackType(PackType), PackNum(PackNum)
        {
            std::memcpy(this->DeviceID, DeviceID, sizeof(this->DeviceID));
        }

        DataHead(size_t PackLen, unsigned short PackType, unsigned char* DeviceID, unsigned char Channel) : PackLen(PackLen), PackType(PackType), UnUsed(Channel)
        {
            std::memcpy(this->DeviceID, DeviceID, sizeof(this->DeviceID));
        }

        DataHead(const DataHead&) = default;
        DataHead& operator=(const DataHead&) = default;
    };

    struct DataEnd
    {
        unsigned short DeviceID = 1;
        double Longitude = 104.2;
        double Latitude = 32.8;
        double Height = 504;

        DataEnd(unsigned short DeviceID) : DeviceID(DeviceID) {}
        DataEnd(const DataEnd&) = default;
        DataEnd& operator=(const DataEnd&) = default;
    };

    struct ParamPowerWB
    {
        long long Time; // ��ʼʱ��	INT64	8		�����������ݶ�Ӧʱ�������������ʱ�̣��߾���ʱ���ʽ����2.1.2.1��
        float Resolution; // Ƶ�ʷֱ���	FLOAT	4	KHz
        int DataPoint; // FFT���� 	INT32	4		FFT����
        int SamplePoint; // ���������� 	INT32	4		FFT���������������
        short Window; // ���������� 	INT16	2		��B.26
        long long StartFreq; // ��ʼƵ��	INT64	8	Hz
        long long StopFreq; // ��ֹƵ��	INT64	8	Hz
        int ChannelNum; // �ŵ�����	INT32	4		��Ƶ�ʷֱ��ʾ����������ŵ������ŵ��������ŵ����� = ����ֹƵ�� - ��ʼƵ�ʣ� / Ƶ�ʷֱ��� + 1

        ParamPowerWB() = default;
        ParamPowerWB(const ParamPowerWB&) = default;
        ParamPowerWB& operator=(const ParamPowerWB&) = default;
    };

    struct StructNBWaveZCResult
    {
        long long StartTime;
        int NanoSeconds;
        long long Frequency; // Hz
        int BandWidth; // Hz
        int Sps;
        char Accuracy = 1; // 0: Byte; 1: Int16; 2: Int32; 3: Float
        char ChannelNum = 2; // 1:Real; 2: Real-Imagine
        short DataPoint = 2048; // Default 2048

        StructNBWaveZCResult() = default;
        StructNBWaveZCResult(const StructNBWaveZCResult&) = default;
        StructNBWaveZCResult& operator=(const StructNBWaveZCResult&) = default;

        void SetNBWaveResultFrequency(unsigned long long Frequency)
        {
            this->Frequency = Frequency;
        }

        void SetNBWaveResultBandWidth(unsigned int BandWidth)
        {
            this->BandWidth = BandWidth;
            Sps = BandWidth / 2048;
        }
    };

    struct StructCheckCXResult
    {
        unsigned int Task = 400;
        int SignalNum;
        short CXType = 6;
        short CXResolution = 1;
        short CXMode = 0;

        StructCheckCXResult() = default;
        StructCheckCXResult(const StructCheckCXResult&) = default;
        StructCheckCXResult& operator=(const StructCheckCXResult&) = default;
    };

    struct StructControlRev
    {
        unsigned int Task = 400;
        short ControlFlag;
        short ErrorMsg;

        StructControlRev(unsigned int Task, short ControlFlag, short ErrorMsg) : Task(Task), ControlFlag(ControlFlag), ErrorMsg(ErrorMsg) {}
    };

    struct StructWorkCommandRev
    {
        unsigned int Task;
        short Data = 0;
        short Detect = 0;
        float FreqRes;
        int SimBW;
        short GMode = 0;
        short MGC;
        short AGC;
        short SmNum;
        short SmMode = 0;
        short LmMode;
        short LmVal;
        short RcvMode;
    };

    struct StructDeviceScheckRev
    {
        unsigned int Task = 400;
        unsigned char DeviveChNum;
        unsigned int ScheckResult;
        unsigned char AGroupNum;
        unsigned int AScheckResult;
    };

#pragma pack()

    struct StructNetData
    {
        int type;
        size_t length = 0;
        char* data = nullptr;

        StructNetData(int type, size_t length) : type(type), length(length), data(new char[length]) {}
        ~StructNetData()
        {
            if (data != nullptr)
                delete[] data;
        }
    };

    void DataHeadToByte(unsigned short, size_t, char*, unsigned short);
    void DataHeadToByte(unsigned short, size_t, char*);
    void DataHeadToByte(unsigned short, size_t, char*, unsigned char);
    void DataEndToByte(char*);
}

#endif
