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
            struct SelfCheck
            {
                unsigned short Ctrl;
                char Reserved[8];
            } SelfCheck;

            struct PulseDemodeParam
            {
                unsigned short Channel;
                unsigned short Ctrl;
                unsigned short Gate;
                unsigned short PulseWidthDepress;
                unsigned short PulseRangeDepress;
            } PulseDemodeParam;

            struct FFT_Param
            {
                char DataPoints;
                char PlaceHolder_;
                char Smooth;
                char SmoothLog;
                char WinType;
                char Reserved[5];
            } FFT_Param;

            struct WB_DDC_Param
            {
                unsigned int CenterFreq;
                char CIC;
                char Reserved[5];
            } WB_DDC_Param;

            struct Rf_Param
            {
                char DataType;
                union Function
                {
                    char Desc;
                    char Mode;
                    char Reserved;
                } Function;
                char Reserved[8];
            } Rf_Param;
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