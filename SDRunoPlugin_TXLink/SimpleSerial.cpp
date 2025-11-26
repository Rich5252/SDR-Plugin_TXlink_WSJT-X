//#include "stdafx.h"
#include "SimpleSerial.h"
#include "MessageBox.h"

SimpleSerial::SimpleSerial(const std::string com_port, DWORD COM_BAUD_RATE, bool bAsync)
{
	if (!bAsync)
	{
		CreateSerialPort(com_port, COM_BAUD_RATE);
	}
	else
	{
		CreateSerialPortAsync(com_port, COM_BAUD_RATE);
	}
}


void SimpleSerial::CustomSyntax(string syntax_type) {

	ifstream syntaxfile_exist("syntax_config.txt");

	if (!syntaxfile_exist) {		
		ofstream syntaxfile;
		syntaxfile.open("syntax_config.txt");

		if (syntaxfile) {
			syntaxfile << "json { }\n";
			syntaxfile << "greater_less_than < >\n";
			syntaxfile.close();
		}
	}

	syntaxfile_exist.close();
	
	ifstream syntaxfile_in;
	syntaxfile_in.open("syntax_config.txt");
	
	string line;
	bool found = false;	

	if (syntaxfile_in.is_open()) {

		while (syntaxfile_in) {			
			syntaxfile_in >> syntax_name_ >> front_delimiter_ >> end_delimiter_;
			getline(syntaxfile_in, line);			
			
			if (syntax_name_ == syntax_type) {
				found = true;
				break;
			}
		}

		syntaxfile_in.close();

		if (!found) {
			syntax_name_ = "";
			front_delimiter_ = ' ';
			end_delimiter_ = ' ';
			std::cout << "Warning: Could not find delimiters, may cause problems!\n";
		}
	}
	else
	{
		printf("Warning: No file open");
	}
}

int32_t SimpleSerial::DataAvailable() {
	//returns number of bytes available in the input buffer or -1 if failed to clear communication errors
	COMSTAT com_stat;
	DWORD errors;

	// Clear communication errors and get the current status
	if (ClearCommError(Serial_HANDLE, &errors, &com_stat)) {
		// Check if there are any bytes waiting in the input buffer
		return com_stat.cbInQue;
	}
	else {
		std::cerr << "SimpleSerial::DataAvailable() Failed to clear communication errors on Port " << com_port_ << std::endl;
		return -1;
	}
}

string SimpleSerial::ReadSerialPortChar()
{
	OVERLAPPED overlapped = { 0 };
	DWORD bytes_read;
	char inc_msg[1];
	string complete_inc_msg = "";

	if (ReadFile(Serial_HANDLE, inc_msg, 1, &bytes_read, NULL))
	{
		return complete_inc_msg.append(inc_msg, 1);
	}
	else
	{
		ClearComm();
		return "";
	}	
}

string SimpleSerial::ReadSerialPortCharAsync()
{
	OVERLAPPED overlapped = { 0 };
	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);  // Create an event for the overlapped structure

	if (overlapped.hEvent == NULL) {
		printf("Error: Failed to create event for overlapped I/O.\n");
		return "";
	}

	DWORD bytes_read = 0;
	char inc_msg[1] = { 0 };
	string complete_inc_msg = "";

	BOOL result = ReadFile(Serial_HANDLE, inc_msg, 1, &bytes_read, &overlapped);

	if (!result && GetLastError() == ERROR_IO_PENDING) {
		// Wait for the asynchronous I/O to complete
		WaitForSingleObject(overlapped.hEvent, INFINITE);
		GetOverlappedResult(Serial_HANDLE, &overlapped, &bytes_read, TRUE);
	}

	if (bytes_read > 0) {
		complete_inc_msg.append(inc_msg, 1);
	}

	CloseHandle(overlapped.hEvent);  // Clean up the event handle

	if (bytes_read > 0) {
		return complete_inc_msg;
	}
	else {
		ClearComm();
		return "";
	}
}

DWORD	SimpleSerial::ClearComm()
{
	ErrorMsg = "";
	ClearCommError(Serial_HANDLE, &errors_, &status_);
	return status_.cbInQue;
}

string SimpleSerial::ReadSerialPort(int reply_wait_time, string syntax_type) {

	DWORD bytes_read;
	char inc_msg[1];	
	string complete_inc_msg;
	bool began = false;

	CustomSyntax(syntax_type);

	unsigned long start_time = (unsigned long) time(nullptr);

	ClearCommError(Serial_HANDLE, &errors_, &status_);	

	while ((time(nullptr) - start_time) < reply_wait_time) {

		if (status_.cbInQue > 0) {
			
			if (ReadFile(Serial_HANDLE, inc_msg, 1, &bytes_read, NULL)) {
				
				if (inc_msg[0] == front_delimiter_ || began) {
					began = true;

					if (inc_msg[0] == end_delimiter_)
						return complete_inc_msg;

					if (inc_msg[0] != front_delimiter_)
						complete_inc_msg.append(inc_msg, 1);
				}				
			}
			else
			{
				return "Warning: Failed to receive data.\n";
			}
		}
	}
	return complete_inc_msg;		
}

bool SimpleSerial::WriteSerialPort(std::string data_sent)
{
	DWORD bytes_sent = 0;
	bool	bCommError = false;

	if (Serial_HANDLE != io_handler_chk) {
		std::cout << get_millis() << "  *********** ERROR: SimpleSerial::WriteSerialPort io_handler invalid, sending: " << data_sent << std::endl;
		connected_ = false;
		return false;
		//Serial_HANDLE = io_handler_chk;
	}
	bCommError = ClearCommError(Serial_HANDLE, &errors_, &status_);
	if (!bCommError) {
		std::cout << get_millis() << "SimpleSerial::WriteSetialPort() ClearCommError #1 failed on port " << com_port_ << "\n";
		PrintCommErrorInfo(errors_);
		connected_ = false;
		return false;
	}
	
	if (!WriteFile(Serial_HANDLE, data_sent.c_str(), data_sent.length(), &bytes_sent, NULL)) {
		std::cout << get_millis() << "SimpleSerial::WriteSetialPort() Write failed of " << data_sent << " ... Attempting to clear port " << com_port_ << "\n";
		if (!ClearCommError(Serial_HANDLE, &errors_, &status_)) {
			std::cout << get_millis() << "SimpleSerial::WriteSetialPort() ClearCommError #2 failed on port " << com_port_ << "\n";
			PrintCommErrorInfo(errors_);
			connected_ = false;
		}
		return false;
	}
	else
	{
		std::cout << get_millis() << "SimpleSerial::WriteSetialPort() Write success: " << data_sent << "  PreBufQ = " << status_.cbOutQue <<std::endl;
		return true;
	}
}

bool SimpleSerial::CloseSerialPort()
{
	if (connected_) {
		connected_ = false;
		FlushFileBuffers(Serial_HANDLE);					// Flush the I/O buffer before closing
		CloseHandle(Serial_HANDLE);
		std::cout << get_millis() << "SimpleSerial::CloseSetialPort() port Closed " << com_port_ << "\n";
		return true;

	}	
	else
	{
		std::cout << get_millis() << "SimpleSerial::CloseSetialPort() port already Closed " << com_port_ << "\n";
		return false;
	}
}

bool SimpleSerial::Connected()
{
	return connected_;
}

SimpleSerial::~SimpleSerial()
{
	if (connected_) {
		connected_ = false;
		CloseHandle(Serial_HANDLE);		
	}
}

void SimpleSerial::ReOpenPort(const std::string com_port, DWORD COM_BAUD_RATE, bool bAsync)
{
	ClearComm();
	CloseSerialPort();
	if (!bAsync)
	{
		CreateSerialPort(com_port, COM_BAUD_RATE);
	}
	else
	{
		CreateSerialPortAsync(com_port, COM_BAUD_RATE);
	}
}


void SimpleSerial::CreateSerialPort(const std::string com_port, DWORD COM_BAUD_RATE)
{
	connected_ = false;
	com_port_ = com_port; // Store the com_port for later use

	Serial_HANDLE = CreateFileA(com_port.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	DWORD inputBufferSize = 4096; // Example buffer size
	DWORD outputBufferSize = 4096; // Example buffer size
	SetupComm(Serial_HANDLE, inputBufferSize, outputBufferSize);

	if (Serial_HANDLE == INVALID_HANDLE_VALUE) {

		if (GetLastError() == ERROR_FILE_NOT_FOUND)
			std::cout << "Warning: Handle was not attached. Reason: %s not available " << com_port << std::endl;
	}
	else {
		io_handler_chk = Serial_HANDLE; // Save the valid handle for future checks

		DCB dcbSerialParams = { 0 };

		if (!GetCommState(Serial_HANDLE, &dcbSerialParams)) {

			std::cout << "Warning: Failed to get current serial params " << com_port << std::endl;
		}

		else {
			dcbSerialParams.BaudRate = COM_BAUD_RATE;
			dcbSerialParams.ByteSize = 8;
			dcbSerialParams.StopBits = ONESTOPBIT;
			dcbSerialParams.Parity = NOPARITY;
			dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

			if (!SetCommState(Serial_HANDLE, &dcbSerialParams))
				std::cout << "Warning: could not set serial port params " << com_port << std::endl;
			else {
				connected_ = true;
				PurgeComm(Serial_HANDLE, PURGE_RXCLEAR | PURGE_TXCLEAR);
				std::cout << get_millis() << "SimpleSerial: successfully opened or reopened " << com_port << std::endl;
			}
		}
	}
}

void SimpleSerial::CreateSerialPortAsync(const std::string com_port, DWORD COM_BAUD_RATE)
{
	connected_ = false;

	Serial_HANDLE = CreateFileA(
		com_port.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);

	if (Serial_HANDLE == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
			string ErrorMsg = "Warning: Handle was not attached. Reason: " + string(com_port) + " not available\n";
			std::cout << ErrorMsg << std::endl;
		return;
	}

	DWORD inputBufferSize = 4096; // Example buffer size
	DWORD outputBufferSize = 4096; // Example buffer size
	SetupComm(Serial_HANDLE, inputBufferSize, outputBufferSize);

	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(DCB); // Initialize the DCB length

	if (!GetCommState(Serial_HANDLE, &dcbSerialParams)) {
		std::cout << "Warning: Failed to get current serial params " << com_port << std::endl;
		CloseHandle(Serial_HANDLE); // Clean up
		return;
	}

	dcbSerialParams.BaudRate = COM_BAUD_RATE;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

	if (!SetCommState(Serial_HANDLE, &dcbSerialParams)) {
		std::cout << "Warning: could not set serial port params " << com_port << std::endl;
		CloseHandle(Serial_HANDLE); // Clean up
		return;
	}

	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = 0; // Maximum time allowed between two bytes
	timeouts.ReadTotalTimeoutConstant = 1; // Total timeout for read operations
	timeouts.ReadTotalTimeoutMultiplier = 0; // Multiplier for read operations
	timeouts.WriteTotalTimeoutConstant = 50; // Total timeout for write operations
	timeouts.WriteTotalTimeoutMultiplier = 10; // Multiplier for write operations

	if (!SetCommTimeouts(Serial_HANDLE, &timeouts)) {
		std::cout  << "Warning: could not set timeouts " << com_port << std::endl;
		CloseHandle(Serial_HANDLE); // Clean up
		return;
	}

	connected_ = true;
	PurgeComm(Serial_HANDLE, PURGE_RXCLEAR | PURGE_TXCLEAR);
}

string SimpleSerial::GetErrorMsg()
{
	return ErrorMsg;
}

void SimpleSerial::PrintCommErrorInfo(DWORD errors_) {
	if (errors_ == 0) {
		std::cout << "No communication errors reported." << std::endl;
		return;
	}

	std::cout << "Communication Error(s) detected (Code: 0x" << std::hex << errors_ << std::dec << "):" << std::endl;

	if (errors_ & CE_RXOVER) {
		std::cout << "  - CE_RXOVER: Input buffer overflow (no room in buffer)." << std::endl;
	}
	if (errors_ & CE_OVERRUN) {
		std::cout << "  - CE_OVERRUN: Character-buffer overrun (next character lost)." << std::endl;
	}
	if (errors_ & CE_RXPARITY) {
		std::cout << "  - CE_RXPARITY: Hardware detected a parity error." << std::endl;
	}
	if (errors_ & CE_FRAME) {
		std::cout << "  - CE_FRAME: Hardware detected a framing error." << std::endl;
	}
	if (errors_ & CE_BREAK) {
		std::cout << "  - CE_BREAK: Hardware detected a break condition." << std::endl;
	}
	if (errors_ & CE_TXFULL) {
		std::cout << "  - CE_TXFULL: Output buffer was full when trying to transmit." << std::endl;
	}
	if (errors_ & CE_MODE) {
		std::cout << "  - CE_MODE: Requested mode is not supported or handle is invalid." << std::endl;
	}
	// Add checks for other less common CE_* flags as needed (CE_IOE, etc.)
}