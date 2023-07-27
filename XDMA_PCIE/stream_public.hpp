#ifndef _STREAM_PUBLIC_H
#define _STREAM_PUBLIC_H

#include <iostream>
#include <thread>

#include "dllexport.h"

#ifdef XDMA_PCIE_WIN_LIB
#include "struct_device_win.h"
#include "stream_dma_win.hpp"
#elif defined XDMA_PCIE_UNIX_LIB
#include "struct_device_unix.h"
#include "stream_dma_unix.hpp"
#endif

bool isRunning = true;

void SetIsRunning(bool state)
{
    isRunning = state;
}

void ReadThread(xdma_device& dev, P_CXDATA_CALLBACK& CallBackCX, P_ZCDATA_CALLBACK& CallBackZC, std::thread& CX_Thread, std::thread& ZC_Thread)
{
    auto ThreadProcDataCX = [&](int channel, const size_t PACK_LEN, const int PACK_NUM)
    {
        const size_t BLOCK_LEN = PACK_LEN * PACK_NUM;
        while (isRunning)
        {
            size_t bytes_remaining = BLOCK_LEN;
            auto ptr = std::make_unique<Struct_Datas<DataWB_FFT>>(PACK_NUM);
            auto buffer = (char*)ptr->ptr;
            try
            {
                while (bytes_remaining > 0)
                {
                    size_t offset = BLOCK_LEN - bytes_remaining;
                    size_t read_len = dev.read_from_engine(buffer + offset, bytes_remaining, channel);
                    if (read_len > 0)
                    {
                        bytes_remaining -= read_len;
                    }
                }
                if (CallBackCX != nullptr)
                {
                    CallBackCX(ptr);
                }
            }
            catch (const std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }
        }
    };
    auto ThreadProcDataZC = [&](int channel, const size_t PACK_LEN, const int PACK_NUM)
    {
        const size_t BLOCK_LEN = PACK_LEN * PACK_NUM;
        while (isRunning)
        {
            size_t bytes_remaining = BLOCK_LEN;
            auto ptr = std::make_unique<Struct_Datas<DataWB_FFT>>(PACK_NUM);
            auto buffer = (char*)ptr->ptr;
            try
            {
                while (bytes_remaining > 0)
                {
                    size_t offset = BLOCK_LEN - bytes_remaining;
                    size_t read_len = dev.read_from_engine(buffer + offset, bytes_remaining, channel);
                    if (read_len > 0)
                    {
                        bytes_remaining -= read_len;
                    }
                }
                if (CallBackZC != nullptr)
                {
                    CallBackZC(ptr);
                }
            }
            catch (const std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }
        }
    };
    try
    {
        CX_Thread = std::thread(ThreadProcDataCX, 0, sizeof(DataWB_FFT), 1);
        ZC_Thread = std::thread(ThreadProcDataZC, 1, sizeof(DataWB_FFT), 1);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

#endif
