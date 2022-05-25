// SerialForWindowsTerminal.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "SerialForWindowsTerminal.h"
#include <vector>
#include <string>
#include <iostream>
#include <boost/asio.hpp> 
#include <boost/asio/windows/stream_handle.hpp>

#define VERSION "1.0"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInstance;
boost::system::error_code ec;
boost::asio::io_service ioctx;
boost::asio::serial_port serialPort(ioctx);

using PortsArray = std::vector<std::pair<std::wstring, int>>;

void SetSerialConfig(int argc, const char *argv[])
{
	std::string serialArgs[6] = { "COM1","9600","8","none","none","none" };
	serialArgs[0] = argv[1];
	int count = 2;

	while (count < argc) {
		if (strcmp(argv[count], "-br") == 0 || strcmp(argv[count], "--baud_rate") == 0) {
			if (++count < argc) {
				serialPort.set_option(boost::asio::serial_port::baud_rate(std::atoi(argv[count])), ec);
				serialArgs[1] = argv[count];
			}
			else {
				serialPort.set_option(boost::asio::serial_port::baud_rate(9600), ec);
			}
		}
		else if (strcmp(argv[count], "-cs") == 0 || strcmp(argv[count], "--character_size") == 0) {
			if (++count < argc) {
				serialPort.set_option(boost::asio::serial_port::character_size(atol(argv[count])), ec);
				serialArgs[2] = argv[count];
			}
			else {
				serialPort.set_option(boost::asio::serial_port::character_size(8), ec);
			}
		}
		else if (strcmp(argv[count], "-sb") == 0 || strcmp(argv[count], "--stop_bit") == 0) {
			if (++count < argc) {
				if (strcmp(argv[count], "none")) {
					serialPort.set_option(boost::asio::serial_port::parity(boost::asio::serial_port::parity::none), ec);
				}
				else if (strcmp(argv[count], "onepointfive")) {
					serialPort.set_option(boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::onepointfive), ec);
				}
				else if (strcmp(argv[count], "two")) {
					serialPort.set_option(boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::two), ec);
				}
				serialArgs[3] = argv[count];
			}
			else {
				serialPort.set_option(boost::asio::serial_port::parity(boost::asio::serial_port::parity::none), ec);
			}

		}
		else if (strcmp(argv[count], "-p") == 0 || strcmp(argv[count], "--parity") == 0) {
			if (++count < argc) {
				if (strcmp(argv[count], "none")) {
					serialPort.set_option(boost::asio::serial_port::parity(boost::asio::serial_port::parity::none), ec);
				}
				else if (strcmp(argv[count], "odd")) {
					serialPort.set_option(boost::asio::serial_port::parity(boost::asio::serial_port::parity::odd), ec);
				}
				else if (strcmp(argv[count], "even")) {
					serialPort.set_option(boost::asio::serial_port::parity(boost::asio::serial_port::parity::even), ec);
				}
				serialArgs[4] = argv[count];
			}
			else {
				serialPort.set_option(boost::asio::serial_port::parity(boost::asio::serial_port::parity::none), ec);
			}
		}
		else if (strcmp(argv[count], "-fc") == 0 || strcmp(argv[count], "--flow_control") == 0) {
			if (++count < argc) {
				if (strcmp(argv[count], "none")) {
					serialPort.set_option(boost::asio::serial_port::flow_control(boost::asio::serial_port::flow_control::none), ec);
				}
				else if (strcmp(argv[count], "software")) {
					serialPort.set_option(boost::asio::serial_port::flow_control(boost::asio::serial_port::flow_control::software), ec);
				}
				else if (strcmp(argv[count], "hardware")) {
					serialPort.set_option(boost::asio::serial_port::flow_control(boost::asio::serial_port::flow_control::hardware), ec);
				}
				serialArgs[5] = argv[count];
			}
			else {
				serialPort.set_option(boost::asio::serial_port::flow_control(boost::asio::serial_port::flow_control::none), ec);
			}
		}
		else if (strcmp(argv[count], "-h") == 0 || strcmp(argv[count], "--help") == 0) {
			std::cout << "--baud_rate\t-br\t\t 波特率" << std::endl;
			std::cout << "--character_size\t-cs\t\t 数据位" << std::endl;
			std::cout << "--stop_bit\t-sb\t\t 停止位[none,onepointfive,two]" << std::endl;
			std::cout << "--parity\t\t-p\t\t 检验模式[none,odd,even]" << std::endl;
			std::cout << "--flow_control\t-fc\t\t 流控模式[none,software,hardware]" << std::endl;

			return;
		}
		count++;
	}
	std::cout << "port\t\t\t" << serialArgs[0] << std::endl;
	std::cout << "baud rate\t\t" << serialArgs[1] << std::endl;
	std::cout << "word length\t\t" << serialArgs[2] << std::endl;
	std::cout << "stop bit\t\t" << serialArgs[3] << std::endl;
	std::cout << "parity\t\t\t" << serialArgs[4] << std::endl;
	std::cout << "flow control\t\t" << serialArgs[5] << std::endl;
}



template <class TStream1, class TStream2>
static void DoStreamToStream(TStream1 &stream1, TStream2 &stream2, std::vector<uint8_t> &buffer)
{
	stream1.async_read_some(
		boost::asio::buffer(buffer.data(), buffer.size()),
		[&stream1, &stream2, &buffer](const boost::system::error_code &ec, std::size_t bytes_transferred)
		{
			//ctrl Q 推出程序
			if (buffer.data()[0] == 17) {
				ioctx.stop();
			}
			//ctrl D 断开串口连接
			if (buffer.data()[0] == 4) {

			}
			//ctrl R 重新连接串口
			if (buffer.data()[0] == 18) {

			}
			if (ec)
			{
				std::cerr << "\033[31m" << "error : " << ec.message() << "\033[0m" << std::endl;
			}
			else
			{
				boost::asio::async_write(
					stream2,
					boost::asio::const_buffer(buffer.data(), bytes_transferred),
					[&stream1, &stream2, &buffer](const boost::system::error_code &ec, std::size_t bytes_transferred)
					{
						if (ec)
						{
							std::cerr << "\033[31m" << "error : " << ec.message() << "\033[0m" << std::endl;
						}
						else
						{
							DoStreamToStream(stream1, stream2, buffer);
						}
					}
				);
			}
		}
	);
}

static boost::system::error_code DoWork()
{
	boost::system::error_code ec;
	boost::asio::windows::stream_handle stdinput(ioctx);
	boost::asio::windows::stream_handle stdoutput(ioctx);
	const auto kBufferSize = 1024;
	std::vector<uint8_t> serialPortRecvBuffer;
	std::vector<uint8_t> serialPortSendBuffer;
	serialPortRecvBuffer.resize(kBufferSize);
	serialPortSendBuffer.resize(kBufferSize);

	auto conin = CreateFile(L"CONIN$", FILE_GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
	auto conout = CreateFile(L"CONOUT$", FILE_GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

	if (stdinput.assign(conin, ec))
		return ec;

	if (stdoutput.assign(conout, ec))
		return ec;

	DoStreamToStream(serialPort, stdoutput, serialPortRecvBuffer);
	DoStreamToStream(stdinput, serialPort, serialPortSendBuffer);
	ioctx.run(ec);
	return ec;
}

void ArgHelp() {
	std::cout << "SerialForWindwosTerminal <command>" << std::endl;
	std::cout << "help\t-h\t\t 显示帮助" << std::endl;
	std::cout << "version\t-v\t\t 显示程序版本" << std::endl;
	std::cout << "<port>\t\t\t\t 要连接的串口号" << std::endl;
}

int main(int argc, const char *argv[])
{
	if (argc > 1)
	{
		if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "help") == 0) {
			ArgHelp();
		}
		else if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "version") == 0) {
			std::cout << VERSION << std::endl;
		}
		else {

			hInstance = GetModuleHandle(nullptr);

			std::cout << "welcome use serial for windows terminal!" << std::endl;

			DWORD consoleMode = 0;
			auto conin = GetStdHandle(STD_INPUT_HANDLE);
			auto conout = GetStdHandle(STD_OUTPUT_HANDLE);
			GetConsoleMode(conin, &consoleMode);
			consoleMode |= ENABLE_MOUSE_INPUT;
			consoleMode &= ~ENABLE_ECHO_INPUT;
			consoleMode &= ~ENABLE_PROCESSED_INPUT;
			consoleMode &= ~ENABLE_LINE_INPUT;
			consoleMode |= ENABLE_QUICK_EDIT_MODE;
			consoleMode |= ENABLE_WINDOW_INPUT;
			consoleMode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
			SetConsoleMode(conin, consoleMode);

			GetConsoleMode(conout, &consoleMode);
			SetConsoleMode(conout, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT);

			auto hWndParent = ::GetForegroundWindow();
			if (hWndParent == nullptr)
				hWndParent = GetConsoleWindow();

			if (serialPort.open(argv[1], ec))
			{
				std::cerr << "\033[31m" << "can not open " << argv[1] << "\033[0m" << std::endl;
				std::cerr << "\033[31m" << "error : " << ec.message() << "\033[0m" << std::endl;
			}
			SetSerialConfig(argc, argv);

			//串口shell交互功能
			ec = DoWork();
			if (ec)
			{
				std::cerr << "\033[31m" << "error : " << ec.message() << "\033[0m" << std::endl;
				return ec.value();
			}
		}
	}
	else
	{
		ArgHelp();
	}


	return ERROR_SUCCESS;
}