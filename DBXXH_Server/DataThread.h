#ifndef DATATHREAD_H
#define DATATHREAD_H

#include "TcpSocket.h"

namespace DBXXH
{
	void DataDealWB(TcpSocket&);
	void DataDealNB(TcpSocket&);
    long long timeConvert(unsigned long long);
}

#endif // DATATHREAD_H
