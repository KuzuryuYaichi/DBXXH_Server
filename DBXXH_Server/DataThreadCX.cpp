#include "StructNetData.h"
#include "TcpSocket.h"
#include "ThreadSafeQueue.h"
#include "StructData.h"
#include "StructDatas.h"
#include "DataThread.h"
#include "PrintHelper.h"

extern DBXXH::threadsafe_queue<std::unique_ptr<Struct_Datas<DataWB_FFT>>> tsqueueCXs;

void DBXXH::TcpSocket::PowerWBDataReplay(const ParamPowerWB& ReplayParm, const std::unique_ptr<StructNetData>& res, size_t Datalen, unsigned short PackNum)
{
    DataHeadToByte(0x0515, Datalen, res->data, PackNum);
    *(ParamPowerWB*)(res->data + sizeof(DataHead)) = ReplayParm;
    DataEndToByte(res->data + Datalen - sizeof(DataEnd));
    SendMsg(res);
}

float DBXXH::ResolveResolution(unsigned char Resolution)
{
    switch (Resolution)
    {
    case 10: return 25.0;
    case 11: return 12.5;
    case 12: return 6.25;
    case 13: return 3.125;
    default: return 0;
    }
}

long long DBXXH::timeConvert(unsigned long long t)
{
    auto UnixTimeToFileTime = [](time_t tmUnixTime) -> long long
    {
        static constexpr long long EPOCH_DIFF = 116444736000000000; //FILETIME starts from 1601-01-01 UTC, epoch from 1970-01-01
        static constexpr long long RATE_DIFF = 10000000;
        long long ll = tmUnixTime * RATE_DIFF + EPOCH_DIFF;
        FILETIME FileTime;
        FileTime.dwLowDateTime = (DWORD)ll;
        FileTime.dwHighDateTime = ll >> 32;
        return *(long long*)&FileTime;
    };

    auto year = (t >> 58) & 0xFF;
    if (year < 23)
        return UnixTimeToFileTime(time(nullptr));
    year += 100;
    unsigned long long month = 0;
    int dayOffset = (t >> 49) & 0x1FF;
    auto hour = (((t >> 48) & 0x1) ? 12 : 0) + ((t >> 44) & 0xF);
    auto minute = (t >> 38) & 0x3F;
    auto second = (t >> 32) & 0x3F;
    int millisecond = (t & 0xFFFFFFFF) / 102.4;

    static short MONTH_DAYS[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    MONTH_DAYS[1] = ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) ? 29 : 28;
    for (month = 0; dayOffset > 0; ++month)
    {
        if (dayOffset - MONTH_DAYS[month] > 0)
            dayOffset -= MONTH_DAYS[month];
        else
        {
            ++month;
            break;
        }
    }

    std::tm tTime;
    tTime.tm_year = 70 + year;
    tTime.tm_mon = month - 1;
    tTime.tm_mday = dayOffset;
    //tTime.tm_yday = days;
    //tTime.tm_wday = 0;
    tTime.tm_hour = hour;
    tTime.tm_min = minute;
    tTime.tm_sec = second;

    return UnixTimeToFileTime(std::mktime(&tTime));
}

void DBXXH::DataDealCX(TcpSocket& socket)
{
    auto ToPowerWB = [&](const DataWB_FFT& recvData)
    {
        auto& ParamPowerWB = g_Parameter.m_ParamPowerWB;
        const auto PerDataLen = sizeof(char) * ParamPowerWB.DataPoint,
            DataLen = PerDataLen,
            Datalen = sizeof(DataHead) + sizeof(ParamPowerWB) + DataLen + sizeof(DataEnd);
        auto res = std::make_unique<StructNetData>(0, Datalen);

        const auto LENGTH = ParamPowerWB.DataPoint - 1;
        auto Data = recvData.Data;
        auto start = res->data + sizeof(DataHead) + sizeof(ParamPowerWB);
        //for (auto g = 0; g < ParamPowerWB.CXGroupNum; ++g)
        //{
            auto& StartTime = *(long long*)start;
            auto Range = start + sizeof(long long);
            for (int p = 0; p < LENGTH; ++p)
            {
                Range[p] = std::max(Data[p] / 10 + 12, 0);
            }
            Range[LENGTH] = Range[LENGTH - 1];
        //    start += CXPerDataLen;
        //}
        socket.PowerWBDataReplay(ParamPowerWB, res, Datalen, 0);
    };

    auto GetQueueDataFun = [&](const DataWB_FFT& recvData)
    {
        switch (g_Parameter.DataType)
        {
        case 1: //WB
        {
            auto& ParamPowerWB = g_Parameter.m_ParamPowerWB;
            std::lock_guard<std::mutex> lk(g_Parameter.ParamPowerWBMutex);
            //if (recvData.Params.DataType != 3 /*|| ParamPowerWB.Resolution != ResolveResolution(recvData.Params.Resolution)*/)
            //    return;
            ToPowerWB(recvData);
            break;
        }
        default:
            return;
        }
    };

    while (true)
    {
        auto ptr = tsqueueCXs.wait_and_pop();
        for (int i = 0; i < ptr->PACK_NUM; ++i)
        {
            GetQueueDataFun(ptr->ptr[i]);
        }
    }
};
