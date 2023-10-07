#include "StructNetData.h"
#include "TcpSocket.h"
#include "ThreadSafeQueue.h"
#include "StructData.h"
#include "StructDatas.h"
#include "DataThread.h"
#include "PrintHelper.h"

extern DBXXH::threadsafe_queue<std::unique_ptr<Struct_Datas<DataNB_Data>>> tsqueueZCs;

void DBXXH::TcpSocket::NBDataReplay(const StructNBWave& ReplayParm, const std::unique_ptr<StructNetData>& res, size_t Datalen, unsigned char Channel)
{
    DataHeadToByte(0x0602, Datalen, res->data, Channel);
    *(StructNBWave*)(res->data + sizeof(DataHead)) = ReplayParm;
    DataEndToByte(res->data + Datalen - sizeof(DataEnd));
    SendMsg(res);
}

void DBXXH::TcpSocket::PulseDataReplay(const std::unique_ptr<StructNetData>& res, size_t Datalen)
{
    DataHeadToByte(0x0605, Datalen, res->data);
    DataEndToByte(res->data + Datalen - sizeof(DataEnd));
    SendMsg(res);
}

void DBXXH::DataDealZC(TcpSocket& socket)
{
    auto ToWaveData = [&](const DataNB_Data& recvData)
    {
        static unsigned char PackIndexAll[PARAMETER_SET::ZC_CH_NUM] = { 0 };
        static std::unique_ptr<StructNetData> resAll[PARAMETER_SET::ZC_CH_NUM] = { nullptr };

        auto& PackIndex = PackIndexAll[recvData.Params.ChNum];
        auto& res = resAll[recvData.Params.ChNum];
        const auto DataLen = sizeof(DataHead) + sizeof(StructNBWave) + PARAMETER_SET::ZC_CH_NUM * sizeof(DataNB_Data::DDCData) + sizeof(DataEnd);

        if (PackIndex == 0 || res == nullptr)
        {
            res = std::make_unique<StructNetData>(0, DataLen);
        }

        auto& NBWaveCXResult = g_Parameter.m_NBWave[recvData.Params.ChNum];
        const auto LENGTH = recvData.DDC_LENGTH;
        auto Data = recvData.DDCData;
        const auto DataBase = (DDC*)(res->data + sizeof(DataHead) + sizeof(StructNBWave) + PackIndex * sizeof(DataNB_Data::DDCData));

        for (int p = 0; p < LENGTH; ++p)
        {
            DataBase[p] = Data[p];
        }

        if (++PackIndex == 8)
        {
            PackIndex = 0;
            if (recvData.Params.Demod == 0x01)
            {
                NBWaveCXResult.AM_DataMax = recvData.Params.NBParams.AM_Params.AM_DataMax;
                NBWaveCXResult.AM_DC = recvData.Params.NBParams.AM_Params.AM_DC;
            }
            NBWaveCXResult.DataType = recvData.Params.Demod;
            socket.NBDataReplay(NBWaveCXResult, res, DataLen, recvData.Params.ChNum);
        }
    };

    auto ToPulseData = [&](const DataNB_Data& recvData)
    {
        static unsigned char PackIndex = 0;
        static std::unique_ptr<StructNetData> res = nullptr;
        const auto LENGTH = recvData.PULSE_LENGTH;
        auto Data = recvData.PulseData;
        const auto DataLen = sizeof(DataHead) + PARAMETER_SET::ZC_CH_NUM * sizeof(DataNB_Data::PulseData) + sizeof(DataEnd);

        if (PackIndex == 0 || res == nullptr)
        {
            res = std::make_unique<StructNetData>(0, DataLen);
        }

        const auto DataBase = (Pulse*)(res->data + sizeof(DataHead) + PackIndex * sizeof(DataNB_Data::PulseData));
        for (int p = 0; p < LENGTH; ++p)
        {
            DataBase[p] = Data[p];
        }

        if (++PackIndex == 8)
        {
            PackIndex = 0;
            socket.PulseDataReplay(res, DataLen);
        }
    };

    auto DataFilter = [&](const DataNB_Data& recvData)
    {
        if (recvData.Params.DataType == 2)
        {
            if (recvData.Params.ChNum < 0 || recvData.Params.ChNum >= PARAMETER_SET::ZC_CH_NUM)
                return;
            auto& NB_Param = g_Parameter.NB_Params.NB_Param[recvData.Params.ChNum];
            if (NB_Param.DDS != recvData.Params.NBParams.DDS || NB_Param.CIC != recvData.Params.NBParams.CIC || NB_Param.Demod != recvData.Params.Demod)
                return;
            ToWaveData(recvData);
        }
        else if (recvData.Params.DataType == 4)
        {
            ToPulseData(recvData);
        }
    };

    while (true)
    {
        auto ptr = tsqueueZCs.wait_and_pop();
        for (int i = 0; i < ptr->PACK_NUM; ++i)
        {
            if (ptr->ptr[i].Params.Head == 0xABCD1234)
                DataFilter(ptr->ptr[i]);
        }
    }
};