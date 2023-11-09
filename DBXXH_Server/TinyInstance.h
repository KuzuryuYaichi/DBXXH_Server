#ifndef _TINY_INSTANCE_H
#define _TINY_INSTANCE_H

#include "TcpSocket.h"
#include "TinyConfig.h"
#include "boost/asio.hpp"

namespace DBXXH
{
    class TinyInstance
    {
    public:
        TinyInstance();
        void join();

    private:
        TinyConfig tinyConfig;
        TcpSocket ServerSocket;

        std::thread DataThreadWB;
        std::thread DataThreadNB;

        void InitThread();
    };
}

#endif
