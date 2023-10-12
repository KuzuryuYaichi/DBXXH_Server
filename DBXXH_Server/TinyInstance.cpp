#include "TinyInstance.h"
#include "../XDMA_PCIE/dllexport.h"
#include "dllImport.h"
#include "DataThread.h"
#include "PrintHelper.h"

constexpr char CONFIG_FILE[] = "config.ini";

DBXXH::TinyInstance::TinyInstance(): tinyConfig(CONFIG_FILE),
    ServerSocket(tinyConfig.Get_LocalIP(), tinyConfig.Get_DataPort()),
    DataThreadCX(DataDealCX, std::ref(ServerSocket)), DataThreadZC(DataDealZC, std::ref(ServerSocket))
{
    InitThread();
    StructCmdWB PulseDetect(0x21F1);
    PulseDetect.Pulse_Param.Gate = 0b0011 << 12 | 0x3080;
    PulseDetect.SendCXCmd();
    ServerSocket.Run();
}

void DBXXH::TinyInstance::join()
{
    if (DataThreadCX.joinable())
        DataThreadCX.join();
    if (DataThreadZC.joinable())
        DataThreadZC.join();
}

void DBXXH::TinyInstance::InitThread()
{
    RegisterCallBackCX(DataCX);
    RegisterCallBackZC(DataZC);
    OpenDevice();
    Printer.PrintInfo();
}
