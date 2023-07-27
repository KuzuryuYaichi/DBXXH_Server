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

    struct StructCmdCX
    {
        unsigned short Head = 0xAA55;
        char ScanSpeed = 4;
        char StateMachine = 0; // 2
        int StartCenterFreq = CENTER_FREQ_KHZ;
        int StopCenterFreq = CENTER_FREQ_KHZ;
        unsigned int AntennaFreq = 530000;
        char RFAttenuation = 10;
        char MFAttenuation = 0;
        char RfMode = 1;
        char CorrectAttenuation = 15;
        unsigned int DDS_CTRL = 0;
        char Resolution = 13;
        char CorrectMode = 1;
        char Smooth = 1;
        char FFT_SCH = 4;
        unsigned short RfProtectTime = 2; // 1/SampleRate
        unsigned short Tail = 0x55AA;

        StructCmdCX() {}

        void SendCXCmd()
        {
            WriteStreamCmd((char*)this, sizeof(StructCmdCX), 0);
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