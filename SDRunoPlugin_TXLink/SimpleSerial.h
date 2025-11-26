#pragma once

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <string.h>
#include <chrono>
#include <thread>
#include <time.h>
#include <fstream>

using namespace std;

class SimpleSerial
{

private:
	HANDLE Serial_HANDLE = nullptr;
	HANDLE io_handler_chk = nullptr;
	COMSTAT status_;
	DWORD errors_;
	string ErrorMsg = "";
	std::string com_port_;

	string syntax_name_;
	char front_delimiter_;
	char end_delimiter_;

	void CustomSyntax(string syntax_type);
	void CreateSerialPort(const std::string com_port, DWORD COM_BAUD_RATE);
	void CreateSerialPortAsync(const std::string com_port, DWORD COM_BAUD_RATE);

public:
	SimpleSerial(const std::string com_port, DWORD COM_BAUD_RATE, bool bAsync);
	~SimpleSerial();

	void ReOpenPort(const std::string com_port, DWORD COM_BAUD_RATE, bool Async);
	DWORD	ClearComm();
	string GetErrorMsg();
	void PrintCommErrorInfo(DWORD errors_);

	int32_t DataAvailable();
	string ReadSerialPort(int reply_wait_time, string syntax_type);
	string ReadSerialPortChar();		//attempt to read a single char, wait until available
	string ReadSerialPortCharAsync();   //attempt to read a single char and return empty if not available
	bool WriteSerialPort(std::string data_sent);
	bool CloseSerialPort();

	bool connected_;
	bool Connected();
};

