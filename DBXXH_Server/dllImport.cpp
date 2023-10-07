#include "dllImport.h"
#include "ThreadSafeQueue.h"
#include "StructData.h"
#include "StructDatas.h"
#include "PrintHelper.h"

DBXXH::threadsafe_queue<std::unique_ptr<Struct_Datas<DataWB_Data>>> tsqueueCXs;
DBXXH::threadsafe_queue<std::unique_ptr<Struct_Datas<DataNB_Data>>> tsqueueZCs;

void DataCX(std::unique_ptr<Struct_Datas<DataWB_Data>>& pBuf_CX)
{
    DBXXH::Printer.AnalyzeCH0(pBuf_CX->PACK_NUM, pBuf_CX->PACK_NUM * sizeof(DataWB_Data));
    tsqueueCXs.push(std::move(pBuf_CX));
}

void DataZC(std::unique_ptr<Struct_Datas<DataNB_Data>>& pBuf_ZC)
{
    DBXXH::Printer.AnalyzeCH1(pBuf_ZC->PACK_NUM, pBuf_ZC->PACK_NUM * sizeof(DataNB_Data));
    tsqueueZCs.push(std::move(pBuf_ZC));
}
