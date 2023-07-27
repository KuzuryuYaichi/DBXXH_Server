#include "PrintHelper.h"

void DBXXH::PrintHelper::PrintInfo()
{
    SpeedAnalyzePCIE = std::thread([this]
    {
        while (true)
        {
            std::printf("\rCH0: %lluB/s CallBack: %llu/s | CH1: %lluB/s CallBack: %llu/s", TransferByte_CX, CX_Count, TransferByte_ZC, ZC_Count);
            CX_Count = 0;
            ZC_Count = 0;
            TransferByte_CX = 0;
            TransferByte_ZC = 0;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
}

void DBXXH::PrintHelper::AnalyzeCH0(int Callback_Count, size_t TransferBytes)
{
    CX_Count += Callback_Count;
    TransferByte_CX += TransferBytes;
}

void DBXXH::PrintHelper::AnalyzeCH1(int Callback_Count, size_t TransferBytes)
{
    ZC_Count += Callback_Count;
    TransferByte_ZC += TransferBytes;
}
