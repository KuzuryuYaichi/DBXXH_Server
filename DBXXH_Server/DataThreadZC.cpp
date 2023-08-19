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
        static constexpr auto MAX_CHANNEL = 8;
        static unsigned char PackIndexAll[MAX_CHANNEL] = { 0 };
        static std::unique_ptr<StructNetData> resAll[MAX_CHANNEL] = { nullptr };

        if (recvData.Params.ChNum < 0 || recvData.Params.ChNum >= MAX_CHANNEL)
            return;
        auto& PackIndex = PackIndexAll[recvData.Params.ChNum];
        auto& res = resAll[recvData.Params.ChNum];
        const auto DataLen = sizeof(DataHead) + sizeof(StructNBWaveZCResult) + MAX_CHANNEL * sizeof(DataNB_DDC::DDCData) + sizeof(DataEnd);

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

        if (++PackIndex == MAX_CHANNEL)
        {
            socket.NBZCDataReplay(NBWaveCXResult, res, DataLen, recvData.Params.ChNum);
            PackIndex = 0;
        }
    };

    while (true)
    {
        auto ptr = tsqueueZCs.wait_and_pop();
        for (int i = 0; i < ptr->PACK_NUM; ++i)
        {
            if (ptr->ptr[i].Params.Head == 0x1234ABCD)
                ToWaveData(ptr->ptr[i]);
        }
    }
};