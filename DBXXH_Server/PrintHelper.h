#pragma once

#include "SingletonHolder.hpp"
#include "StructNetData.h"
#include <thread>

namespace DBXXH
{
	class PrintHelper: public SingletonHolder<PrintHelper>
	{
    public:
        void PrintInfo();
        void AnalyzeCH0(int, size_t);
        void AnalyzeCH1(int, size_t);

    public:
        virtual ~PrintHelper() override = default;

    protected:
        size_t TransferByte_CX = 0, TransferByte_ZC = 0, CX_Count = 0, ZC_Count = 0;
        std::thread SpeedAnalyzePCIE;
	};

    static inline auto& Printer = PrintHelper::get_instance();

    struct PARAMETER_SET: public SingletonHolder<PARAMETER_SET>
    {
    public:
        static constexpr int ZC_CH_NUM = 8;

        unsigned char DeviceID[14];

        struct WB_Params_
        {
            char GainMode = 0;
            char RfGain = 0;
            char DigitGain = 0;
            char Resolution = 13;
            char Smooth = 1;
            char Feedback = 0;
            int NbCenterFreqRF;
        } WB_Params;

        struct NB_Params_
        {
            unsigned short BankNum;
            struct NB_Param_
            {
                unsigned int DDS;
                unsigned short CIC;
                unsigned char Demod;
            } NB_Param[ZC_CH_NUM];
        } NB_Params;
        

        std::mutex ParamPowerWBMutex;
        ParamPowerWB m_ParamPowerWB;
        void SetParamPowerWB(unsigned int Task, long long CenterFreq)
        {
            std::lock_guard<std::mutex> lock(ParamPowerWBMutex);
            //m_ParamPowerWB.Task = Task;
            //m_ParamPowerWB.CenterFreq = CenterFreq;

        }
        void SetParamPowerWB(unsigned int Task, unsigned long Resolution)
        {
            std::lock_guard<std::mutex> lock(ParamPowerWBMutex);
            m_ParamPowerWB.Resolution = Resolution;
            m_ParamPowerWB.ChannelNum = std::pow(2, Resolution);
            switch (Resolution)
            {
            case 0x0E: m_ParamPowerWB.DataPoint = 10240 + 1; break;
            case 0x0D: m_ParamPowerWB.DataPoint = 5120 + 1; break;
            case 0x0C: m_ParamPowerWB.DataPoint = 2560 + 1; break;
            case 0x0B: m_ParamPowerWB.DataPoint = 1280 + 1; break;
            case 0x0A: m_ParamPowerWB.DataPoint = 640 + 1; break;
            }
        }

        std::mutex NBWaveMutex;
        StructNBWaveZCResult m_NBWaveZCResult[ZC_CH_NUM];
        void SetNBWaveResultFrequency(int ChNum, unsigned long long Frequency)
        {
            if (ChNum < 0 || ChNum >= ZC_CH_NUM)
                return;
            std::lock_guard<std::mutex> lock(NBWaveMutex);
            m_NBWaveZCResult[ChNum].SetNBWaveResultFrequency(Frequency);
        }
        void SetNBWaveResultBandWidth(int ChNum, unsigned int BandWidth)
        {
            if (ChNum < 0 || ChNum >= ZC_CH_NUM)
                return;
            std::lock_guard<std::mutex> lock(NBWaveMutex);
            m_NBWaveZCResult[ChNum].SetNBWaveResultBandWidth(BandWidth);
        }
    };

    static inline auto& g_Parameter = PARAMETER_SET::get_instance();
}
