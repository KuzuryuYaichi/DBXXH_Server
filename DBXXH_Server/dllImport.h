#ifndef _DLL_IMPORT_H
#define _DLL_IMPORT_H

#include "../XDMA_PCIE/XDMA_PCIE_global.h"

void Q_DECL_STDCALL DataCX(std::unique_ptr<Struct_Datas<DataWB_Data>>&);

void Q_DECL_STDCALL DataZC(std::unique_ptr<Struct_Datas<DataNB_Data>>&);

#endif
