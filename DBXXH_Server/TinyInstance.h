#ifndef _TINY_INSTANCE_H
#define _TINY_INSTANCE_H

#include "TcpSocket.h"
#include "TinyConfig.h"
#include "boost/asio.hpp"
#include "SerialPortSession.h"

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
        SerialPortSession SerialPort;

        std::thread DataThreadCX;
        std::thread DataThreadZC;

        void InitThread();
    };
}

#endif
