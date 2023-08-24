#include "TcpSession.h"
#include "boost/property_tree/ini_parser.hpp"
#include "boost/property_tree/ptree.hpp"
#include "PrintHelper.h"
#include <regex>

DBXXH::TcpSession::TcpSession(boost::asio::ip::tcp::socket&& socket, std::unordered_set<TcpSession*>& SessionSet, std::mutex& SessionSetMutex) :
    socket(std::move(socket)), SessionSet(SessionSet), SessionSetMutex(SessionSetMutex)
{
    boost::system::error_code err_code;
    //this->socket.set_option(boost::asio::ip::tcp::socket::reuse_address(true), err_code);
    //this->socket.set_option(boost::asio::ip::tcp::no_delay(true), err_code);
    //this->socket.set_option(boost::asio::socket_base::linger(true, 0), err_code);
    //this->socket.set_option(boost::asio::socket_base::keep_alive(true), err_code);
    this->socket.set_option(boost::asio::socket_base::send_buffer_size(4 * 1024 * 1024), err_code);
}

DBXXH::TcpSession::~TcpSession()
{
    RemoveFromSet();
}

void DBXXH::TcpSession::StartRevDataWork()
{
    SendState = true;
}

void DBXXH::TcpSession::StopRevDataWork()
{
    SendState = false;
}

void DBXXH::TcpSession::SetAppConfig()
{
    //boost::property_tree::ptree root_node;
    //boost::property_tree::read_ini("Params.ini", root_node);
    //auto tag = root_node.get_child("Params");
    //tag.put("StateMachine", g_Parameter.StateMachine);
    //tag.put("StartFreq", g_Parameter.StartCenterFreq);
    //tag.put("StopFreq", g_Parameter.StopCenterFreq);
    //tag.put("AntennaFreq", g_Parameter.AntennaFreq);
    //tag.put("RFAttenuation", g_Parameter.RFAttenuation);
    //tag.put("MFAttenuation", g_Parameter.MFAttenuation);
    //tag.put("RfMode", g_Parameter.RfMode);
    //tag.put("CorrectAttenuation", g_Parameter.CorrectAttenuation);
    //tag.put("DDS_CTRL", g_Parameter.DDS_CTRL);
    //tag.put("RFAttenuation", g_Parameter.CmdResolution);
    //tag.put("CorrectMode", g_Parameter.CorrectMode);
    //tag.put("Smooth", g_Parameter.Smooth);
    //boost::property_tree::write_ini("Params.ini", root_node);
}

void DBXXH::TcpSession::StartWork()
{
    auto self(shared_from_this());
    std::thread([self, this]
    {
        while (isRunning)
        {
            read();
        }
        std::cout << "Read Thread Exit" << std::endl;
    }).detach();

    std::thread([self, this]()
    {
        while (isRunning)
        {
            std::printf("\r%llu Packets Transferred\n", TransferByte);
            TransferByte = 0;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        std::cout << "Print Thread Exit" << std::endl;
    }).detach();
}

void DBXXH::TcpSession::AddToSet()
{
    StartWork();
    std::lock_guard<std::mutex> lk(SessionSetMutex);
    SessionSet.emplace(this);
}

void DBXXH::TcpSession::read()
{
    static constexpr int BUFFER_LEN = 1024;
    try
    {
        auto data = std::make_unique<Order>(BUFFER_LEN);
        boost::system::error_code err;
        size_t left = BUFFER_LEN, offset = 0;
        //while (left > 0)
        //{
            auto bytes_transferred = socket.read_some(boost::asio::buffer(data->order + offset, left), err);
            if (err.failed())
            {
                std::cout << "Read Failed: " << err.what() << std::endl;
                isRunning = false;
                return;
            }
            //left -= bytes_transferred;
            //offset += bytes_transferred;
        //}
        RecvCommandFun(data);
    }
    catch (const std::exception& e)
    {
        std::cout << "Read Exception:" << e.what() << std::endl;
        isRunning = false;
    }
}

void DBXXH::TcpSession::write(const std::unique_ptr<StructNetData>& data)
{
    std::lock_guard<std::mutex> lk(WriteLock);
    try
    {
        boost::system::error_code err;
        size_t left = data->length, offset = 0;
        while (left > 0)
        {
            auto bytes_transferred = socket.write_some(boost::asio::buffer(data->data + offset, left), err);
            if (err.failed())
            {
                std::cout << "Write Failed: " << err.what() << std::endl;
                isRunning = false;
                return;
            }
            left -= bytes_transferred;
            offset += bytes_transferred;
        }
        ++TransferByte;
    }
    catch (const std::exception& e)
    {
        std::cout << "Write Exception: " << e.what() << std::endl;
        isRunning = false;
    }
}

void DBXXH::TcpSession::RemoveFromSet()
{
    std::lock_guard<std::mutex> lk(SessionSetMutex);
    if (SessionSet.count(this))
        SessionSet.erase(this);
}

void DBXXH::TcpSession::ControlReplay(unsigned int Task, short ControlFlag, short ErrorMsg)
{
    static constexpr auto DataLen = sizeof(DataHead) + sizeof(StructControlRev);
    auto res = std::make_unique<StructNetData>(1, DataLen);
    DataHeadToByte(0x0500, DataLen, res->data);
    new (res->data + sizeof(DataHead)) StructControlRev(Task, ControlFlag, ErrorMsg);
    write(res);
}

void DBXXH::TcpSession::WorkParmReplay(const StructWorkCommandRev& ReplayParm)
{
    static constexpr auto DataLen = sizeof(DataHead) + sizeof(StructWorkCommandRev);
    auto res = std::make_unique<StructNetData>(1, DataLen);
    DataHeadToByte(0x0511, DataLen, res->data);
    *(StructWorkCommandRev*)(res->data + sizeof(DataHead)) = ReplayParm;
    write(res);
}

void DBXXH::TcpSession::ScheckReplay(const StructDeviceScheckRev& ReplayParm)
{
    static constexpr auto DataLen = sizeof(DataHead) + sizeof(StructDeviceScheckRev);
    auto res = std::make_unique<StructNetData>(1, DataLen);
    DataHeadToByte(0x0512, DataLen, res->data);
    *(StructDeviceScheckRev*)(res->data + sizeof(DataHead)) = ReplayParm;
    write(res);
}

void DBXXH::TcpSession::RecvCommandFun(const std::unique_ptr<Order>& buffer)
{
    auto head = (DataHead*)buffer->order;
    if (head->Head != 0xF99FEFFE || head->PackType != 0x08FE)
    {
        std::cout << "Head Error" << std::endl;
        return;
    }
    auto CommandPack = std::string(buffer->order + sizeof(DataHead), head->PackLen - sizeof(DataHead));

    std::regex re(";|\r\n");
    std::sregex_token_iterator first(CommandPack.begin(), CommandPack.end(), re, -1), last;
    std::vector<std::string> Cmd(first, last);

    if (Cmd.size() == 0)
        return;
    unsigned int TaskValue = 0;
    if (Cmd[0].find_first_of("Task:") != 0)
        return;
    TaskValue = std::stoul(Cmd[0].substr(sizeof("Task")));
    if (Cmd.size() <= 2)
    {
        ControlReplay(TaskValue, 0, 1);
        return;
    }
    if (Cmd[2].find_first_of("Type:") != 0)
        return;
    short TypeValue = std::stoul(Cmd[2].substr(sizeof("Type")), 0, 16);
    switch (TypeValue)
    {
    case 0x0101:
    {
        if (Cmd[3].find_first_of("Scheck:") == 0)
        {
            ControlReplay(TaskValue, 1, 0);
            std::cout << "Type: SelfCheck, Val: Dev State SelfCheck, State: SelfChecking" << std::endl;
            SelfCheck();
        }
        break;
    }
    case 0x0102:
    {
        if (Cmd[3].find_first_of("WorkCtrl:") == 0)
        {
            short WorkCtrlValue = std::stoul(Cmd[3].substr(sizeof("WorkCtrl")));
            switch (WorkCtrlValue)
            {
            case 0:
            {
                ControlReplay(TaskValue, 1, 0);
                std::cout << "Type: WorkCtrl, Val: Work Param Inquire, State: Inquiring" << std::endl;
                ReplayCommand.Task = TaskValue;
                WorkParmReplay(ReplayCommand);
                std::cout << "State: Inquired" << std::endl;
                break;
            }
            case 1:
            {
                std::cout << "Type: WorkCtrl, Val: Work Param Stop, State: Stopping" << std::endl;
                StopRevDataWork();
                ControlReplay(TaskValue, 1, 0);
                std::cout << "State: Stopped" << std::endl;
                break;
            }
            case 2:
            {
                std::cout << "Type: WorkCtrl, Val: Reset, State: Resetting" << std::endl;
                //initWorkCommandRev();
                ControlReplay(TaskValue, 1, 0);
                std::cout << "State: Resetted" << std::endl;
                break;
            }
            default:
            {
                ControlReplay(TaskValue, 0, 0);
                break;
            }
            }
        }
        break;
    }
    case 0x0103:
    {
        SetCmdWBParams(Cmd);
        std::cout << "Type: WorkParam Set, Val: CX Param Set, State: Setting" << std::endl;
        //g_Parameter.SetFixedCXResult(TaskValue, (unsigned int)CmdWB.Context.FFT_Param.DataPoints);
        //g_Parameter.SetSweepCXResult(TaskValue, (unsigned int)CmdWB.Context.FFT_Param.DataPoints);
        //g_Parameter.SetTestCXResult(TaskValue, (unsigned int)CmdWB.Context.FFT_Param.DataPoints);
        ControlReplay(TaskValue, 1, 0);
        ReplayCommand.Task = TaskValue;
        WorkParmReplay(ReplayCommand);
        SetAppConfig();
        break;
    }
    case 0x0403:
    {
        SetCmdNBReceiver(Cmd);
        break;
    }
    case 0x0411:
    {
        SetCmdNBChannel(Cmd);
        break;
    }
    default:
    {
        ControlReplay(TaskValue, 0, 0);
        return;
    }
    }
    //tsqueueCXs.clear();
}

void DBXXH::TcpSession::SelfCheck()
{
    CmdWB.Type = 1;
    CmdWB.Context.SelfCheck.Ctrl = 0xAAAA;
    std::memset(CmdWB.Context.SelfCheck.Reserved, 0, sizeof(CmdWB.Context.SelfCheck.Reserved));
}

void DBXXH::TcpSession::SetCmdWBParams(const std::vector<std::string>& Cmd)
{
    for (size_t n = Cmd.size(), i = 3; i < n; ++i)
    {
        auto index = Cmd[i].find_first_of(':');
        if (index >= 0)
        {
            auto ParamName = Cmd[i].substr(0, index);
            if (ParamName == "CenterFreq")
            {
                CmdWB.Type = 0x23F1;
                auto CenterFreq_MHz = std::stoull(Cmd[i].substr(sizeof("CenterFreq"))) / 1e6;
                CmdWB.Context.WB_DDC_Param.CenterFreq = std::pow(2, 22) * CenterFreq_MHz / 3;
                std::cout << "CenterFreq: " << CenterFreq_MHz << std::endl;
            }
            else if (ParamName == "SimBW")
            {
                CmdWB.Type = 0x23F1;
                auto simBW = std::stoi(Cmd[i].substr(sizeof("SimBW")));
                if (simBW == 1)
                    g_Parameter.Resolution = CmdWB.Context.WB_DDC_Param.CIC = 1;
                else if (simBW == 2)
                    g_Parameter.Resolution = CmdWB.Context.WB_DDC_Param.CIC = 2;
                else if (simBW == 3)
                    g_Parameter.Resolution = CmdWB.Context.WB_DDC_Param.CIC = 3;
                else if (simBW == 4)
                    g_Parameter.Resolution = CmdWB.Context.WB_DDC_Param.CIC = 4;
                else if (simBW == 5)
                    g_Parameter.Resolution = CmdWB.Context.WB_DDC_Param.CIC = 5;
                else if (simBW == 6)
                    g_Parameter.Resolution = CmdWB.Context.WB_DDC_Param.CIC = 6;
                else
                    continue;
                std::cout << "SimulateBandwidth: " << simBW << std::endl;
                CmdWB.SendCXCmd();
                ReplayCommand.SimBW = simBW;
            }
            else if (ParamName == "FreqRes")
            {
                CmdWB.Type = 0x22F1;
                auto FreqResValue = std::stoi(Cmd[i].substr(sizeof("FreqRes")));
                if (FreqResValue == 0x0E)
                    g_Parameter.Resolution = CmdWB.Context.FFT_Param.DataPoints = 0x0E;
                else if (FreqResValue == 0x0D)
                    g_Parameter.Resolution = CmdWB.Context.FFT_Param.DataPoints = 0x0D;
                else if (FreqResValue == 0x0C)
                    g_Parameter.Resolution = CmdWB.Context.FFT_Param.DataPoints = 0x0C;
                else if (FreqResValue == 0x0B)
                    g_Parameter.Resolution = CmdWB.Context.FFT_Param.DataPoints = 0x0B;
                else if (FreqResValue == 0x0A)
                    g_Parameter.Resolution = CmdWB.Context.FFT_Param.DataPoints = 0x0A;
                else
                    continue;
                std::cout << "FreqResolution: " << FreqResValue << std::endl;
                ReplayCommand.FreqRes = FreqResValue;
            }
            else if (ParamName == "SmNum")
            {
                CmdWB.Type = 0x22F1;
                auto SmNumValue = std::stoi(Cmd[i].substr(sizeof("SmNum")));
                if (SmNumValue == 1)
                {
                    g_Parameter.Smooth = CmdWB.Context.FFT_Param.Smooth = 1; CmdWB.Context.FFT_Param.SmoothLog = 0;
                }
                else if (SmNumValue == 2)
                {
                    g_Parameter.Smooth = CmdWB.Context.FFT_Param.Smooth = 2; CmdWB.Context.FFT_Param.SmoothLog = 1;
                }
                else if (SmNumValue == 4)
                {
                    g_Parameter.Smooth = CmdWB.Context.FFT_Param.Smooth = 4; CmdWB.Context.FFT_Param.SmoothLog = 2;
                }
                else if (SmNumValue == 8)
                {
                    g_Parameter.Smooth = CmdWB.Context.FFT_Param.Smooth = 8; CmdWB.Context.FFT_Param.SmoothLog = 3;
                }
                else if (SmNumValue == 16)
                {
                    g_Parameter.Smooth = CmdWB.Context.FFT_Param.Smooth = 16; CmdWB.Context.FFT_Param.SmoothLog = 4;
                }
                else if (SmNumValue == 32)
                {
                    g_Parameter.Smooth = CmdWB.Context.FFT_Param.Smooth = 32; CmdWB.Context.FFT_Param.SmoothLog = 5;
                }
                else
                    continue;
                CmdWB.Context.FFT_Param.PlaceHolder_ = 0;
                CmdWB.SendCXCmd();
                std::cout << "SmoothTime: " << CmdWB.Context.FFT_Param.Smooth;
                ReplayCommand.SmNum = CmdWB.Context.FFT_Param.Smooth;
            }
            else if (ParamName == "GainMode")
            {
                auto GainMode = std::stoi(Cmd[i].substr(sizeof("GainMode")));
                if (GainMode == 0)
                {
                    g_Parameter.GainMode = CmdWB.Context.Rf_Param.DataType = 1;
                    std::cout << "GainMode: AGC" << std::endl;
                }
                else if (GainMode == 1)
                {
                    g_Parameter.GainMode = CmdWB.Context.Rf_Param.DataType = 2;
                    std::cout << "GainMode: MGC" << std::endl;
                }
                else
                    continue;
                CmdWB.Type = 0x27F1;
                ReplayCommand.GainMode = GainMode;
            }
            else if (ParamName == "Rf_MGC")
            {
                auto MGC = std::stoi(Cmd[i].substr(sizeof("Rf_MGC")));
                if (MGC > 31 || MGC < 0)
                    continue;
                CmdWB.Type = 0x26F1;
                CmdWB.Context.Rf_Param.DataType = 1;
                CmdWB.SendCXCmd();
                std::cout << "GainType: MGC, GainValue: " << MGC << std::endl;
                g_Parameter.RfGain = CmdWB.Context.Rf_Param.Value = MGC;
                ReplayCommand.Rf_MGC = MGC;
            }
            else if (ParamName == "Digit_MGC")
            {
                auto MGC = std::stoi(Cmd[i].substr(sizeof("Digit_MGC")));
                if (MGC > 31 || MGC < 0)
                    continue;
                CmdWB.Type = 0x28F1;
                CmdWB.Context.Digit_Param.DataType = 1;
                CmdWB.Context.Digit_Param.Value = MGC;
                CmdWB.SendCXCmd();
                std::cout << "GainType: MGC, GainValue: " << MGC << std::endl;
                g_Parameter.DigitGain = CmdWB.Context.Digit_Param.Value = MGC;
                ReplayCommand.Digit_MGC = MGC;
            }
            else if (ParamName == "Feedback")
            {
                auto Feedback = std::stoi(Cmd[i].substr(sizeof("Feedback")));
                CmdWB.Type = 0x26F1;
                CmdWB.Context.Digit_Param.DataType = 2;
                CmdWB.Context.Digit_Param.Value = Feedback;
                CmdWB.SendCXCmd();
                std::cout << "Feedback: " << Feedback << std::endl;
                g_Parameter.Feedback = Feedback;
            }
        }
    }
}

void DBXXH::TcpSession::SetCmdNBReceiver(const std::vector<std::string>& Cmd)
{
    CmdZC.CmdType = 0;
    for (size_t n = Cmd.size(), i = 3; i < n; ++i)
    {
        auto index = Cmd[i].find_first_of(':');
        if (index >= 0)
        {
            auto ParmName = Cmd[i].substr(0, index);
            if (ParmName == "Freq")
            {
                auto Freq = std::stoul(Cmd[i].substr(sizeof("Freq")));
                CmdZC.CmdRF.RfType = 1;
                CmdZC.CmdRF.RfData = Freq;
                g_Parameter.NbCenterFreqRF = Freq / 1e3;
            } 
        }
    }
    CmdZC.SendZCCmd();
}

void DBXXH::TcpSession::SetCmdNBChannel(const std::vector<std::string>& Cmd)
{
    CmdZC.CmdType = 1;
    for (size_t n = Cmd.size(), i = 3; i < n; ++i)
    {
        auto index = Cmd[i].find_first_of(':');
        if (index >= 0)
        {
            auto ParmName = Cmd[i].substr(0, index);
            if (ParmName == "BankNum")
            {
                auto BankNum = std::stol(Cmd[i].substr(sizeof("BankNum")));
                if (BankNum < 0 || BankNum > 15)
                    return;
                CmdZC.CmdNB.Channel = BankNum;
            }
            else if (ParmName == "Freq")
            {
                auto Freq = std::stoul(Cmd[i].substr(sizeof("Freq")));
                CmdZC.CmdNB.DDS = std::round(std::pow(2, 32) * (Freq - g_Parameter.NbCenterFreqRF) / 250000);
                g_Parameter.SetNBWaveResultFrequency(CmdZC.CmdNB.Channel, Freq * 1e3);
            }
            else if (ParmName == "DDCBW")
            {
                auto DDCBW = std::stol(Cmd[i].substr(sizeof("DDCBW")));
                unsigned short CIC = 8000;
                switch (DDCBW)
                {
                case 2400: CmdZC.CmdNB.CIC = 8000; g_Parameter.SetNBWaveResultBandWidth(CmdZC.CmdNB.Channel, DDCBW); break;
                case 4800: CmdZC.CmdNB.CIC = 4000; g_Parameter.SetNBWaveResultBandWidth(CmdZC.CmdNB.Channel, DDCBW); break;
                case 9600: CmdZC.CmdNB.CIC = 2000; g_Parameter.SetNBWaveResultBandWidth(CmdZC.CmdNB.Channel, DDCBW); break;
                case 19200: CmdZC.CmdNB.CIC = 1000; g_Parameter.SetNBWaveResultBandWidth(CmdZC.CmdNB.Channel, DDCBW); break;
                case 38400: CmdZC.CmdNB.CIC = 500; g_Parameter.SetNBWaveResultBandWidth(CmdZC.CmdNB.Channel, DDCBW); break;
                case 76800: CmdZC.CmdNB.CIC = 250; g_Parameter.SetNBWaveResultBandWidth(CmdZC.CmdNB.Channel, DDCBW); break;
                case 96000: CmdZC.CmdNB.CIC = 200; g_Parameter.SetNBWaveResultBandWidth(CmdZC.CmdNB.Channel, DDCBW); break;
                default: break;
                }
            }
        }
    }
    CmdZC.SendZCCmd();
}
