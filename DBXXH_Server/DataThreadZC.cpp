#include "StructNetData.h"
#include "TcpSocket.h"
#include "ThreadSafeQueue.h"
#include "StructData.h"
#include "StructDatas.h"
#include "DataThread.h"
#include "PrintHelper.h"

extern DBXXH::threadsafe_queue<std::unique_ptr<Struct_Datas<DataNB_DDC>>> tsqueueZCs;

void DBXXH::TcpSocket::NBZCDataReplay(const StructNBWaveZCResult& ReplayParm, const std::unique_ptr<StructNetData>& res, size_t Datalen, unsigned char Channel)
{
    DataHeadToByte(0x0602, Datalen, res->data, Channel);
    *(StructNBWaveZCResult*)(res->data + sizeof(DataHead)) = ReplayParm;
    DataEndToByte(res->data + Datalen - sizeof(DataEnd));
    SendMsg(res);
}

void DBXXH::DataDealZC(TcpSocket& socket)
{
    auto ToWaveData = [&](const DataNB_DDC& recvData)
    {
        static unsigned char PackIndexAll[PARAMETER_SET::ZC_CH_NUM] = { 0 };
        static std::unique_ptr<StructNetData> resAll[PARAMETER_SET::ZC_CH_NUM] = { nullptr };

        auto& PackIndex = PackIndexAll[recvData.Params.ChNum];
        auto& res = resAll[recvData.Params.ChNum];
        const auto DataLen = sizeof(DataHead) + sizeof(StructNBWaveZCResult) + PARAMETER_SET::ZC_CH_NUM * sizeof(DataNB_DDC::DDCData) + sizeof(DataEnd);

        if (PackIndex == 0 || res == nullptr)
        {
            res = std::make_unique<StructNetData>(0, DataLen);
        }

        auto& NBWaveCXResult = g_Parameter.m_NBWaveZCResult[recvData.Params.ChNum];
        const auto LENGTH = recvData.LENGTH;
        auto Data = recvData.DDCData;
        const auto DataBase = (DDC*)(res->data + sizeof(DataHead) + sizeof(StructNBWaveZCResult) + PackIndex * sizeof(DataNB_DDC::DDCData));

        for (int p = 0; p < LENGTH; ++p)
        {
            DataBase[p] = Data[p];
        }

        if (++PackIndex == 8)
        {
            socket.NBZCDataReplay(NBWaveCXResult, res, DataLen, recvData.Params.ChNum);
            PackIndex = 0;
        }
    };

    auto DataFilter = [&](const DataNB_DDC& recvData)
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
            static int ii = 0;
            ++ii;
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