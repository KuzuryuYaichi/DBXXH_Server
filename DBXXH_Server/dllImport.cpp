#include "dllImport.h"
#include "ThreadSafeQueue.h"
#include "StructData.h"
#include "StructDatas.h"
#include "PrintHelper.h"

DBXXH::threadsafe_queue<std::unique_ptr<Struct_Datas<DataWB_FFT>>> tsqueueCXs;
DBXXH::threadsafe_queue<std::unique_ptr<Struct_Datas<DataNB_DDC>>> tsqueueZCs;

void DataCX(std::unique_ptr<Struct_Datas<DataWB_FFT>>& pBuf_CX)
{
    DBXXH::Printer.AnalyzeCH0(pBuf_CX->PACK_NUM, pBuf_CX->PACK_NUM * sizeof(DataWB_FFT));
    tsqueueCXs.push(std::move(pBuf_CX));
}

void DataZC(std::unique_ptr<Struct_Datas<DataNB_DDC>>& pBuf_ZC)
{
    DBXXH::Printer.AnalyzeCH1(pBuf_ZC->PACK_NUM, pBuf_ZC->PACK_NUM * sizeof(DataNB_DDC));
    tsqueueZCs.push(std::move(pBuf_ZC));
}
