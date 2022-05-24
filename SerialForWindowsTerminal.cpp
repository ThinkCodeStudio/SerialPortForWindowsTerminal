// SerialForWindowsTerminal.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "SerialForWindowsTerminal.h"
#include <vector>
#include <string>
#include <iostream>
#include <boost/asio.hpp> 
#include <boost/asio/windows/stream_handle.hpp>

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInstance;
boost::system::error_code ec;
boost::asio::io_service ioctx;
boost::asio::serial_port serialPort(ioctx);

using PortsArray = std::vector<std::pair<std::wstring, int>>;

void SetSerialConfig(int argc, const char *argv[], boost::system::error_code &ec)
{
	int count = 2;

	while (count < argc) {
		if (strcmp(argv[count], "-br") == 0 || strcmp(argv[count], "--baud_rate") == 0) {
			if (++count < argc) {
				serialPort.set_option(boost::asio::serial_port::baud_rate(std::atoi(argv[count])), ec);
			}
			else {
				serialPort.set_option(boost::asio::serial_port::baud_rate(9600), ec);
			}
		}
		else if (strcmp(argv[count], "-wl") == 0 || strcmp(argv[count], "--word_length") == 0) {
			if (++count < argc) {
				serialPort.set_option(boost::asio::serial_port::character_size(atol(argv[count])), ec);
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
			}
			else {
				serialPort.set_option(boost::asio::serial_port::flow_control(boost::asio::serial_port::flow_control::none), ec);
			}
		}
		count++;
	}
}

template <class TStream1, class TStream2>
static void DoStreamToStream(TStream1 &stream1, TStream2 &stream2, std::vector<uint8_t> &buffer)
{
	stream1.async_read_some(
		boost::asio::buffer(buffer.data(), buffer.size()),
		[&stream1, &stream2, &buffer](const boost::system::error_code &ec, std::size_t bytes_transferred)
		{
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

static boost::system::error_code DoWork(boost::asio::io_service &ioctx, boost::asio::serial_port &serialPort)
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
	std::cout << "welcome use serial for windows terminal!" << std::endl;
	ioctx.run(ec);
	return ec;
}

int main(int argc, const char *args[])
{
	hInstance = GetModuleHandle(nullptr);

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
	if (argc > 1)
	{
		if (serialPort.open(args[1], ec))
		{
			std::cerr << "\033[31m" << "can not open " << args[1] << "\033[0m" << std::endl;
			std::cerr << "\033[31m" << "error : " << ec.message() << "\033[0m" << std::endl;
		}
		SetSerialConfig(argc, args, ec);
	}
	else
	{
		return ERROR_CANCELLED;
	}

	//串口shell交互功能
	ec = DoWork(ioctx, serialPort);
	if (ec)
	{
		std::cerr << "\033[31m" << "error : " << ec.message() << "\033[0m" << std::endl;
		return ec.value();
	}
	return ERROR_SUCCESS;
}