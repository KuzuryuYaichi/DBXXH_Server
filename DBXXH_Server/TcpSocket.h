#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <iostream>
#include <algorithm>
#include <thread>
#include <regex>
#include <unordered_set>
#include "boost/asio.hpp"
#include "TcpSession.h"

namespace DBXXH
{
	class TcpSocket
	{
	public:
		explicit TcpSocket(const std::string&, const unsigned short&);
		void SendMsg(const std::unique_ptr<StructNetData>&);
		void WBDataReplay(const ParamPowerWB&, const std::unique_ptr<StructNetData>&, size_t, unsigned short);
		void NBDataReplay(const StructNBWave&, const std::unique_ptr<StructNetData>&, size_t, unsigned char);
		void PulseDataReplay(const std::unique_ptr<StructNetData>&, size_t);
		void Run();

	private:
		static constexpr int MAX_CONNECTIONS = 1;
		boost::asio::io_service ioService;
		boost::asio::ip::tcp::acceptor m_acceptor;
		boost::asio::ip::tcp::socket m_socket;
		std::unordered_set<TcpSession*> SessionSet;
		std::mutex SessionSetMutex;
		boost::asio::ip::tcp::endpoint GetHostAddress(const boost::asio::ip::tcp::endpoint&);
		void async_accept();
	};
}

#endif
