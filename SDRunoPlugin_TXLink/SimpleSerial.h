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
	HANDLE io_handler_;
	COMSTAT status_;
	DWORD errors_;

	string syntax_name_;
	char front_delimiter_;
	char end_delimiter_;

	void CustomSyntax(string syntax_type);
	void CreateSerialPort(char* com_port, DWORD COM_BAUD_RATE);

public:
	SimpleSerial(char* com_port, DWORD COM_BAUD_RATE);
	void ReOpenPort(char* com_port, DWORD COM_BAUD_RATE);
	DWORD	ClearComm();

	string ReadSerialPort(int reply_wait_time, string syntax_type);
	string ReadSerialPortChar();								//attempt to read a single char and return empty if not available
	bool WriteSerialPort(char *data_sent);
	bool CloseSerialPort();
	~SimpleSerial();
	bool connected_;
	bool Connected();
};

