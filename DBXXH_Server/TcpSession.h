#ifndef TCP_SESSION_H
#define TCP_SESSION_H

#include <memory>
#include <thread>
#include <unordered_set>
#include "boost/asio.hpp"
#include "StructCmd.hpp"
#include "ThreadSafeQueue.h"
#include "StructNetData.h"

namespace DBXXH
{
	class TcpSession : public std::enable_shared_from_this<TcpSession>
	{
	public:
		TcpSession(boost::asio::ip::tcp::socket&&, std::unordered_set<TcpSession*>&, std::mutex&);
		TcpSession(const TcpSession&) = delete;
		TcpSession& operator=(const TcpSession&) = delete;
		TcpSession(TcpSession&&) = default;
		TcpSession& operator=(TcpSession&&) = default;
		~TcpSession();
		void read();
		void write(const std::unique_ptr<StructNetData>&);
		void RecvCommandFun(const std::unique_ptr<Order>&);
		void AddToSet();
		void RemoveFromSet();
		bool SendState = true;
		boost::asio::ip::tcp::socket socket;

	protected:
		void StartWork();

	private:
		bool isRunning = true;
		std::unordered_set<TcpSession*>& SessionSet;
		std::mutex& SessionSetMutex;
		std::mutex WriteLock;
		unsigned int TaskValue = 0;
		StructCmdWB CmdWB;
		StructCmdZC CmdZC;
		StructWorkCommandRev ReplayCommand;
		StructDeviceScheckRev ReplayScheck;
		size_t TransferByte = 0;

		void SelfCheck();
		void SetCmdWBParams(const std::vector<std::string>& Cmd);
		void SetCmdNBChannel(const std::vector<std::string>&);

		void StartRevDataWork();
		void StopRevDataWork();
		void SetAppConfig();

		void ControlReplay(unsigned int, short, short);
		void WorkParmReplay(const StructWorkCommandRev&);
		void ScheckReplay(const StructDeviceScheckRev&);
	};
}

#endif
