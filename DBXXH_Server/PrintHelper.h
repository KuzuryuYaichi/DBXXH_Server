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
        static constexpr int CX_CH_NUM = 4;
        static constexpr int ZC_CH_NUM = 16;
        static constexpr int CALC_MASK()
        {
            int res = 0;
            for (int i = 0; i < CX_CH_NUM; ++i)
            {
                res |= 1 << i;
            }
            return res;
        }

        unsigned char DeviceID[14];
        long long StartCenterFreq = CENTER_FREQ_HZ;
        long long StopCenterFreq = CENTER_FREQ_HZ;
        char RFAttenuation = 10;
        char MFAttenuation = 0;
        char RfMode = 0;
        char CorrectAttenuation = 15;
        char Resolution = 13;
        char Smooth = 1;
        int NbCenterFreqRF;

        enum DATA_TRANS
        {
            TEST_CHANNEL = 0,
            CX_WB,
            CX_NB,
            CX_SWEEP
        };
        DATA_TRANS DataType = CX_WB;

        std::mutex ParameterMutex;
        void SetCmd(long long CmdStartCenterFreq, long long CmdStopCenterFreq)
        {
            std::lock_guard<std::mutex> lock(ParameterMutex);
            this->StartCenterFreq = CmdStartCenterFreq;
            this->StopCenterFreq = CmdStopCenterFreq;
        }

        std::mutex ParamPowerWBMutex;
        ParamPowerWB m_ParamPowerWB;
        void SetParamPowerWB(unsigned int Task, long long CenterFreq)
        {
            std::lock_guard<std::mutex> lock(ParamPowerWBMutex);
            //m_ParamPowerWB.Task = Task;
            //m_ParamPowerWB.CenterFreq = CenterFreq;

        }
        void SetParamPowerWB(unsigned int Task, unsigned int Resolution)
        {
            std::lock_guard<std::mutex> lock(ParamPowerWBMutex);
            switch (Resolution)
            {
            case 10: m_ParamPowerWB.Resolution = 25.0; break;
            case 11: m_ParamPowerWB.Resolution = 12.5; break;
            case 12: m_ParamPowerWB.Resolution = 6.25; break;
            case 13: m_ParamPowerWB.Resolution = 3.125; break;
            default: break;
            }
            m_ParamPowerWB.ChannelNum = std::pow(2, 13 - Resolution);
            m_ParamPowerWB.DataPoint = BAND_WIDTH_KHZ / m_ParamPowerWB.Resolution + 1;
        }

        std::mutex NBWaveMutex;
        StructNBWaveZCResult NBWaveZCResult[ZC_CH_NUM];
        void SetNBWaveResultFrequency(int ChNum, unsigned long long Frequency)
        {
            if (ChNum < 0 || ChNum >= ZC_CH_NUM)
                return;
            std::lock_guard<std::mutex> lock(NBWaveMutex);
            NBWaveZCResult[ChNum].SetNBWaveResultFrequency(Frequency);
        }
        void SetNBWaveResultBandWidth(int ChNum, unsigned int BandWidth)
        {
            if (ChNum < 0 || ChNum >= ZC_CH_NUM)
                return;
            std::lock_guard<std::mutex> lock(NBWaveMutex);
            NBWaveZCResult[ChNum].SetNBWaveResultBandWidth(BandWidth);
        }

        char isTestingInner = CALC_MASK();
        char isTestingOuter = CALC_MASK();
        bool SelfTestInner[CX_CH_NUM] = { false };
        bool SelfTestOuter[CX_CH_NUM] = { false };
    };

    static inline auto& g_Parameter = PARAMETER_SET::get_instance();
}
