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
        union 
        {
            struct SelfCheck_
            {
                unsigned short Ctrl;
                char Reserved[8];
            } SelfCheck;

            struct Pulse_Param_
            {
                unsigned short Channel;
                unsigned short Gate;
                unsigned short WidthDepress;
                unsigned short AmplDepress;
                char Reserved[2];
            } Pulse_Param;

            struct FFT_Param_
            {
                char DataPoints;
                char PlaceHolder_;
                char Smooth;
                char SmoothLog;
                char WinType;
                char Reserved[5];
            } FFT_Param;

            struct WB_DDC_Param_
            {
                unsigned int CenterFreq;
                char CIC;
                char Reserved[5];
            } WB_DDC_Param;

            struct Rf_Param_
            {
                char Type;
                char State;
                char Value[4];
                char Reserved[4];
            } Rf_Param;

            struct Digit_Param_
            {
                char Type;
                char Value;
                char Reserved[8];
            } Digit_Param;
        };

        unsigned short Tail = 0x5FF5;

        StructCmdWB() {}

        StructCmdWB(unsigned short Type): Type(Type) {}

        void SendCXCmd()
        {
            WriteStreamCmd((char*)this, sizeof(StructCmdWB), 0);
        }
    };

    struct StructCmdNB
    {
        unsigned char Head[6] = { (unsigned char)0xFA, (unsigned char)0xA5, (unsigned char)0xFB, (unsigned char)0xB5, (unsigned char)0xFC, (unsigned char)0xC5 };
        unsigned short Type;
        union
        {
            struct DDC_Param_
            {
                unsigned short Channel;
                unsigned short CIC;
                unsigned char DemodType;
                unsigned int DDS;
                char ButterFly = 0x22;
            } DDC_Param;

            struct Demod_Param_
            {
                unsigned char Channel_SSB;
                unsigned int SSB_DDS;
                unsigned char Channel_CW;
                unsigned int CW_DDS;
            } Demod_Param;

            struct PSK_Params_
            {
                unsigned char Channel;
                unsigned char PSK_Rate;
                unsigned char Reserved[8];
            } PSK_Params;
        };
        
        unsigned short Tail = 0x5FF5;

        StructCmdNB() {}

        StructCmdNB(unsigned short Type) : Type(Type) {}

        void SendZCCmd()
        {
            WriteStreamCmd((char*)this, sizeof(StructCmdNB), 0);
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