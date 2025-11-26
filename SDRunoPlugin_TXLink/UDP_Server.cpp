#include <iostream>
#include <winsock2.h>
#include <vector>
#include <cstring>
#include <format>
#include <tuple>
#include <chrono>
#include <thread>
#include <windows.h>
#include <atomic>

#include <sstream>     // For std::stringstream (to build the hex string efficiently)
#include <iomanip>     // For std::hex, std::uppercase, std::setw, std::setfill
#include <cstdint>     // For uint8_t (optional, but good practice for clarity)

#include "WSJTX_Packet.h"
#include "UDP_Server.h"
#include "MessageQueue.h"


#define debugUDP

using namespace std;

#pragma comment(lib,"ws2_32.lib") // Winsock Library
#pragma warning(disable:4996) 

//UDP port defaults for wsjtx
#define BUFLEN 512
#define PORT 2237
#define SERVER "127.0.0.1"  // or "localhost" - ip address of UDP server

//externals that provide the simple comms to the plugin
std::atomic <bool> UDPExitRequest = false;
std::atomic <bool> UDPevent = false;          //set to indicate new status update
std::mutex UDPEvent_mtx;

ThreadSafeMessageQueue<std::string> WSJTX_UDPInQ;            //TODO data from wsjtx to plugin
ThreadSafeMessageQueue<uint64_t> WSJTX_UDPOutQ;           //data from plugin to wsjtx (SetFreq)

bool UDPHaltTXevent = false;    // force immediate TX halt
bool TXflg = false;
int TXDF = 0;
std::string WSJTX_TXmodeStr = "FT8";
std::string LastTXmsg = ""; 
bool TXmodeChange = true;
std::string DXcall = "";
std::string DXgrid = "";
std::string newDXcall = "";
std::string strReport = "";
uint32_t RxDF = 0;
std::string strDecodeMsg = "NOCALL";
uint64_t curFrequency = 0;                  //local store
uint64_t curTXFrequency = 0;                  //from form
uint64_t newFrequency = 0;                  // non zero ifchanged (extern)

std::string FT8TXsymbols = "3140652002457262353553516142250015633140652505007562410457700470324420543140652";       //"NOCALL"
std::string FT8TxNoSymbols = "0000000000000000000000000000000000000000000000000000000000000000000000000000000";
std::string strFT8WSJTXpath = "C:\\WSJT\\wsjtx\\bin\\ft8code.exe";         // must use double \\ (or /). This is emcapsulated in "" so can have spaces.
                                                                        // path is stored in SDRuno ini file (wsjtxPath) and can be modified manually if required.

std::string FT4TXsymbols = "001321033112330313210320122110210110232220103032312323100312203003023102222320131323332231310212123032010";     //"NOCALL"
std::string FT4TxNoSymbols = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
std::string strFT4WSJTXpath = "C:\\WSJT\\wsjtx\\bin\\ft4code.exe";         // must use double \\ (or /). This is emcapsulated in "" so can have spaces.

std::string TXsymbols = "3140652002457262353553516142250015633140652505007562410457700470324420543140652";       //"NOCALL" default
std::string TxNoSymbols = FT8TxNoSymbols;                                           //default in case of error or unsupported modes
std::string strWSJTXpath = "C:\\WSJT\\wsjtx\\bin\\ft8code.exe";

std::string strCallsign = "G4AHN";
std::string strLocator = "IO91";

std::mutex m_lock2;

LPWSTR ConvertToLPWSTR(const char* charString);
SOCKET server_socket;

void dump_packet_hex(const std::vector<uint8_t>& packet) {
    std::stringstream ss;
    ss << std::hex << std::uppercase;

    for (size_t i = 0; i < packet.size(); ++i) {
        ss << std::setw(2) << std::setfill('0') << (int)packet[i] << " ";
        if ((i + 1) % 16 == 0) {
            ss << "\n";
        }
    }
    std::cout << "\n--- INCOMING HEX DUMP (" << packet.size() << " bytes) ---\n" << ss.str() << std::endl;
}


//wsjtx UDP datagram server with parser and ft8 encoding using wsjtx ft8code.exe utility
//this server is invoked into a seperate thread from the TXlink plugin for SDRuno app.
// status update to the plugin is via the simple extern data values above.
void UDP_Server()
{
    sockaddr_in server{}, client{};

    // initialise winsock
    WSADATA wsa;
    std::cout << get_millis() << "Initialising Winsock..." << std::endl;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        std::cout << get_millis() << "Failed. Error Code: " << WSAGetLastError() << std::endl;
        exit(0);
    }
    std::cout << get_millis() << "Initialised." << std::endl;

    // create a socket
  
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        std::cout << get_millis() << "Could not create socket: " << WSAGetLastError() << std::endl;
    }
    std::cout << get_millis() << "Socket created." << std::endl;

    // prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER);
    server.sin_port = htons(PORT);

    // bind
    if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
    {
        std::cout << get_millis() << "Bind failed with error code: " << WSAGetLastError() << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << get_millis() << "Bind done." << std::endl;

    // Set a receive timeout (e.g., 500 milliseconds)
    // The timeout value is in milliseconds for SO_RCVTIMEO
    int timeout_ms = 500;
    if (setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms)) == SOCKET_ERROR)
    {
        std::cout << get_millis() << "setsockopt(SO_RCVTIMEO) failed with error code: " << WSAGetLastError() << std::endl;
        // Decide if you want to exit or log a warning and continue without timeout
    }

	int waitCount = 0;
    while (!UDPExitRequest)
    {
#ifdef debugUDP
        //if (waitCount == 0) { std::cout << std::endl << "Waiting for data... " << waitCount << std::endl; }
#endif
        //fflush(stdout);
        char message[BUFLEN] = {};

        // try to receive some data, this is a blocking call
        int message_len;
        int slen = sizeof(sockaddr_in);

		// Use recvfrom with a timeout
        message_len = recvfrom(server_socket, message, BUFLEN, 0, (sockaddr*)&client, &slen);
        if (message_len == SOCKET_ERROR)
        {
            int lastError = WSAGetLastError();
            if (lastError == WSAETIMEDOUT)
            {
                // This is the expected timeout error, just continue the loop
                // to check UDPExitRequest. No message was received.
#ifdef debugUDP
				if (waitCount == 0) { std::cout << std::endl << "UDP waiting : "; }
                std::cout << " " << ++waitCount;
                //if ((++waitCount % 5) == 0) { std::cout << " " << waitCount; }
#endif
                continue; // Go back to the top of the loop to check UDPExitRequest
            }
            else
            {
                // A real error occurred (not just a timeout)
                std::cout << std::endl << "recvfrom() failed with error code: " << lastError << std::endl;
                // Consider more robust error handling than exit(0)
                // Maybe set UDPExitRequest = true to gracefully exit the loop
                // Or log the error and break; to ensure cleanup
                break; // Exit the loop on critical error
            }
        }
        else if (message_len == 0)
        {
            // Connection gracefully closed (less common for UDP, but good to handle)
            std::cout << std::endl << "recvfrom() returned 0 (connection closed)." << std::endl;
            break; // Exit the loop
        }
        else
        {
			if (waitCount > 0) { std::cout << std::endl; }
            // Message received successfully
            if (UDPExitRequest) {
                std::cout << get_millis() << "UDP Exit Request received, exiting server loop without processing received packet." << std::endl;
                break; // Exit the loop if exit request is set
			}

#ifdef debugUDPx
            std::cout << std::endl << get_millis() << "Received packet from " << inet_ntoa(client.sin_addr) << " " << ntohs(client.sin_port) << "Len: " << message_len << std::endl;
			
#endif
            waitCount = 0; // Reset wait count on successful receive
			std::string strPacketType = ProcessPacket(message, message_len);
			std::cout << get_millis() << "UDP packet : " << strPacketType << std::endl;
        }

        // OUTGOING MESSAGES
        // --- 2. Check internal queue and send any pending messages ---
        while (!WSJTX_UDPOutQ.empty()) {
            uint64_t dialFrequencyHz = WSJTX_UDPOutQ.waitAndPop();
            if (!dialFrequencyHz == 0) {
                std::cout << get_millis() << " UDP Server sending message to WSJT-X. Freq = : " << dialFrequencyHz << std::endl;
                sendSetFreqCommand(                                 // Example: Set 14.074 MHz FT8, TX Odd
                    server_socket,
                    "MyClientApp",
                    dialFrequencyHz,
                    true,
                    "FT8",
                    2500,
                    "",
                    false,
                    15,
                    1500,
                    1500,
                    "DX_CALL",
                    "GRID",
                    true,
                    false // TX Odd (Direction = 0)
                );
             }
        }
	}                   //loop until UDPExitRequest is set

	// Cleanup
	std::cout << "\n" << get_millis() << "UDP Server cleanup..." << std::endl;
    closesocket(server_socket);
    WSACleanup();
}

std::string ProcessPacket(char * message, int message_len)
{
    std::vector<uint8_t> pkt(message, message + message_len);
    size_t idx = 0;
    
    WSJTX_Packet NewPacket( pkt, idx);
    NewPacket.Decode();
	//if (NewPacket.PacketType != 1) { return; } //ignore everything except status packets here

#ifdef debugUDPx 
    std::cout << "Type: " << NewPacket.PacketType << "  ClientID: " << NewPacket.ClientID << std::endl;
#endif

    if (NewPacket.PacketType == 0) {
        return "Heartbeat";
        WSJTX_Heartbeat HeartbeatPacket(pkt, NewPacket.index);
        HeartbeatPacket.Decode();
        // emit UI update signal for heartbeat if needed
    }
    else if (NewPacket.PacketType == 1) {
		std::string ret = "Status";

        //std::cout << "Status Packet DUMP" << std::endl;
        //dump_packet_hex(pkt);

        //printHexFrequency(28074000);
        //printHexFrequency(28074000000);

        //std::lock_guard<std::mutex> lock(UDPEvent_mtx);                     // Locks the UDPEvent mutex

        //status packet contains next ft8 message and TX on/off updates
        WSJTX_Status StatusPacket( pkt, NewPacket.index);
        StatusPacket.Decode();
        //std::cout << "Status decode. Frequency = " << StatusPacket.Frequency << " Mode = " << StatusPacket.Mode << " TxEnabled = " << StatusPacket.TxEnabled << std::endl;

        std::string msg = StatusPacket.DefaultTXMessage;
        msg = removeTrailingSpaces(msg);
		if (msg == "") { msg = "CQ G4AHN IO91"; }                           //default message if empty

		
        if (UDPevent) { std::cout << "UDP_Server: ******* WARNING ************  UDP event already set" << std::endl; }
		int sleeps = 0;
        while (UDPevent && sleeps < 50)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));         //wait for previous event to be processed
            sleeps++;
			if (UDPExitRequest) { return "UDP_Server: UDPExitRequest"; } // exit if requested
        } 
        if (sleeps > 0) { std::cout << get_millis() << " UDP event wait: " << sleeps << " times" << std::endl; }
        
        if (StatusPacket.Frequency != curFrequency && newFrequency == 0)
        {
            newFrequency = StatusPacket.Frequency;
            curFrequency = newFrequency;                //reset to zero in form when UDPevent detected
        }
        if (WSJTX_TXmodeStr != StatusPacket.Mode) { TXmodeChange = true; }
        WSJTX_TXmodeStr = StatusPacket.Mode;
        if (TXflg && !StatusPacket.Transmitting && !StatusPacket.TxEnabled) { UDPHaltTXevent = true; }
        TXflg = StatusPacket.Transmitting;  //transmit on/off
        TXDF = StatusPacket.TxDF;           //transmit freq offset added to set TX freq
        if (LastTXmsg == "" || LastTXmsg != msg)
        {
            TXsymbols = EncodeMessage(WSJTX_TXmodeStr, msg);         //list of tones to transmit
        }
        else
        {
            ret = ret + ": Msg unchanged: " + msg;
        }
		LastTXmsg = msg;                                                                        //store last message sent   
        if (StatusPacket.DXCall != DXcall) { newDXcall = StatusPacket.DXCall; }
        DXcall = StatusPacket.DXCall;
		DXgrid = StatusPacket.DXgrid;
		strReport = StatusPacket.Report;    //report to send
		RxDF = StatusPacket.RxDF;       //receive freq offset
        UDPevent = true;                    //flag for main UDP server code to update Arduino
        return ret;
    }
    else if (NewPacket.PacketType == 2) {
        WSJTX_Decode DecodePacket(pkt, NewPacket.index);
        DecodePacket.Decode();
        // Process DecodePacket and emit UI update signal
        return "Decode: " + DecodePacket.Message;
    }
    else if (NewPacket.PacketType == 3) {
        return "Erase";
        WSJTX_Erase ErasePacket(pkt, NewPacket.index);
        ErasePacket.Decode();
        // emit UI erase message signal
    }
    else if (NewPacket.PacketType == 4) {
		return "Reply";
        WSJTX_Reply ReplyPacket(pkt, NewPacket.index);
        ReplyPacket.Decode();
        // Process ReplyPacket and emit UI update signal
    }
    else if (NewPacket.PacketType == 5) {
		return "Logged";
        WSJTX_Logged LoggedPacket(pkt, NewPacket.index);
        LoggedPacket.Decode();
        // emit UI update signal for logged packet
    }
	else if (NewPacket.PacketType == 17) {
		std::cout << "SetFreq Packet DUMP" << std::endl;
        dump_packet_hex(pkt);
        
        WSJTX_SetFreq SetFreqPacket(pkt, NewPacket.index);
		std::cout << "SetFreq Packet Head: ClientID = " << NewPacket.ClientID << " Index after head = " << NewPacket.index << std::endl;
        try {
            SetFreqPacket = WSJTX_SetFreq(pkt, NewPacket.index);
        }
        catch (const std::exception& e) {
            std::cerr << "Exception during WSJTX_SetFreq construction: " << e.what() << std::endl;
			return "SetFreq Error";
		}
        try {
            SetFreqPacket.Decode();
        }
        catch (const std::exception& e) {
            std::cerr << "Exception during WSJTX_SetFreq Decode: " << e.what() << std::endl;
            return "SetFreq Error";
                    }
        // Process SetFreqPacket and emit UI update signal

		std::cout << "SetFreq Packet: ClientID = " << SetFreqPacket.ClientID << " Freq = " << SetFreqPacket.dialFrequency << " Mode = " << SetFreqPacket.Mode << " TxPeriod = " << SetFreqPacket.TxPeriod << std::endl;
	    return "SetFreq";
    }
    std::cout << "Unknown Packet DUMP type " << to_string(NewPacket.PacketType) << std::endl;
    dump_packet_hex(pkt);
	return "Unknown " + to_string(NewPacket.PacketType);
}

std::string removeTrailingSpaces(std::string str) {
    // Find the position of the last non-space character
    auto pos = str.find_last_not_of(' ');

    // Erase all trailing spaces
    if (pos != std::string::npos) {
        str.erase(pos + 1);
    }
    else {
        // The string contains only spaces
        str.clear();
    }
    return str;
}



/*
std::string EncodeMessage(std::string WSJTX_TXmodeStr, std::string strMessage)
{
    // use wsjtx ft8code.exe utility to encode next ft8 message into 79 symbols list (tone numbers)
    // input is the required FT8 message, output is the symbols list (79 chars)
    // the external strDecodeMsg is set to the text message that will be decoded by wsjtx
    // if an error occurs then the symbols are set to "NOCALL" and the strDecodeMsg set to error msg
    // Create a command line string to execute the Consul app

	std::cout << get_millis() << "EncodeMessage start: " << WSJTX_TXmodeStr << " " << strMessage << std::endl;

    TxNoSymbols = FT8TxNoSymbols;
    if (WSJTX_TXmodeStr == "FT4") { TxNoSymbols = FT4TxNoSymbols; }      //TX fixed freq.
    
    //TUNE
    if (strMessage == "TUNE")
    {
        strDecodeMsg = "TUNE";
        return TxNoSymbols;                                         //default is FT8
    }

    std::string command = "\"" + strFT8WSJTXpath + "\"";        //default is FT8
    if (WSJTX_TXmodeStr == "FT4") { command = "\"" + strFT4WSJTXpath + "\""; }
    command = command + " \"" + strMessage + "\"";

    //example messages, see "ft8code.exe -T" for full list encoded by wsjtx
    //command = command + " \"<GM750XXX/P> GM7AHN/P RR73\"";
    //command = command + " \"GM7XXX GM7AHN RR73\"";

    HANDLE g_hChildStd_OUT_Rd = NULL;
    HANDLE g_hChildStd_OUT_Wr = NULL;
    HANDLE g_hChildStd_IN_Rd = NULL;
    HANDLE g_hChildStd_IN_Wr = NULL;
    SECURITY_ATTRIBUTES saAttr{};

    // Set up the security attributes for the pipes
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create pipes for the child process's STDOUT and STDIN
    if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0)) {
        std::cerr << "ftxcode.exe CreatePipe failed." << std::endl;
        strDecodeMsg = "ftxcode.exe CreatePipe failed";
        return TxNoSymbols;
    }

    // Ensure the read and write handles to the pipes for STDOUT and STDIN are not inherited
    if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "SetHandleInformation failed" << std::endl;
        strDecodeMsg = "ftxcode.exe SetHandleInformation failed";
        return TxNoSymbols;
    }

    // Create the child process
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));

    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = g_hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
    siStartInfo.hStdInput = g_hChildStd_IN_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // invoke Windows console application
    if (!CreateProcess(NULL, ConvertToLPWSTR(command.c_str()), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &siStartInfo, &piProcInfo)) {
        std::cerr << "CreateProcess failed" << std::endl;
        strDecodeMsg = "ftxcode.exe CreateProcess failed";
        return TxNoSymbols;
    }

   // Close the write end of the pipe so the child process stops reading
    //CloseHandle(g_hChildStd_IN_Wr);

    // Read the output from the child process's pipe for STDOUT
    DWORD dwRead = 0;
    CHAR chBuf[9182] = "";
    BOOL bSuccess = FALSE;
    std::string strLine = "";
    std::string symbols = "";
    int lineCount = 0;

    //need to read char by char as app hangs if you try and read past last char out
    while ((bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, 1, &dwRead, NULL)) && dwRead != 0) {
        strLine = strLine + std::string(chBuf, dwRead);
        if (strLine.length() > 1 && strLine.substr(strLine.length() - 2) == "\r\n")
        {
            symbols = strLine;
#ifdef debugUDP
            std::cout << strLine;
#endif
            
            lineCount++;
            switch (lineCount) {
            case 1:
                // check first line
                if (!(strLine.length() == 89 && strLine.substr(42, 7) == "Decoded"))
                {
                    strDecodeMsg = "Encode error L1 - " + to_string(strLine.length());
                    return TxNoSymbols;
                }
                break;
            case 3:
                // extract "Decoded" msg which shows what is actually encoded in message symbols
                if (!(strLine.length() >= 98))
                {
                    strDecodeMsg = "Encode error L3 - " + to_string(strLine.length());
                    return TxNoSymbols;
                }
                strDecodeMsg = removeTrailingSpaces(strLine.substr(42, 40));
                break;
            }
            strLine = "";
        }
        if (WSJTX_TXmodeStr == "FT8" && lineCount >= 16) { break; }     // 16th line is the encoded tone list
        if (WSJTX_TXmodeStr == "FT4" && lineCount >= 22) { break; }     // 22th line is the encoded tone list
    }

    //remove spaces and CRLF
    symbols.erase(std::remove(symbols.begin(), symbols.end(), ' '), symbols.end());
    //check and remove cr/lf
    if (WSJTX_TXmodeStr == "FT8" && symbols.length() == 81)
    {
        symbols = symbols.substr(0, 79);
    }
    else if (WSJTX_TXmodeStr == "FT4" && symbols.length() == 107)
    {
        symbols = symbols.substr(0, 105);
    }
    else
    {
        strDecodeMsg = "ftxcode.exe symbols error - " + to_string(symbols.length());
        symbols = TxNoSymbols;
    }
    //strDecodeMsg = to_string(symbols.length());

    // Close process and thread handles
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);

#ifdef debugUDP
    std::cout << "\nDone: " << "symbols length = " << symbols.length() << "  strDecodeMsg = " << strDecodeMsg << std::endl;
    std::cout << symbols << std::endl;
#endif
    return symbols;
}
*/

std::string EncodeMessage(const std::string& WSJTX_TXmodeStr, const std::string& strMessage) {
    std::cout << get_millis() << "EncodeMessage start: " << WSJTX_TXmodeStr << " " << strMessage << std::endl;

    const std::string ft8Path = strFT8WSJTXpath;
    const std::string ft4Path = strFT4WSJTXpath;

    std::string symbols;
    size_t requiredLength = 0;

    if (WSJTX_TXmodeStr == "FT8") {
        symbols = FT8TxNoSymbols;
        requiredLength = 79;
    }
    else if (WSJTX_TXmodeStr == "FT4") {
        symbols = FT4TxNoSymbols;
        requiredLength = 105;
    }
    else {
        strDecodeMsg = "Invalid WSJTX_TXmodeStr";
        return TxNoSymbols;
    }

    if (strMessage == "TUNE") {
        strDecodeMsg = "TUNE";
        return symbols;
    }

    // --- Command setup with wstring for Unicode API ---
    std::wstring command;
    if (WSJTX_TXmodeStr == "FT8") {
        command = L"\"" + std::wstring(ft8Path.begin(), ft8Path.end()) + L"\"";
    }
    else if (WSJTX_TXmodeStr == "FT4") {
        command = L"\"" + std::wstring(ft4Path.begin(), ft4Path.end()) + L"\"";
    }
    command += L" \"" + std::wstring(strMessage.begin(), strMessage.end()) + L"\"";

    // --- Handle and pipe setup ---
    HANDLE hChildStdOutRead = NULL;
    HANDLE hChildStdOutWrite = NULL;

    SECURITY_ATTRIBUTES saAttr{};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hChildStdOutRead, &hChildStdOutWrite, &saAttr, 0)) {
        strDecodeMsg = "CreatePipe failed.";
        return symbols;
    }
    if (!SetHandleInformation(hChildStdOutRead, HANDLE_FLAG_INHERIT, 0)) {
        strDecodeMsg = "SetHandleInformation failed.";
        CloseHandle(hChildStdOutRead);
        CloseHandle(hChildStdOutWrite);
        return symbols;
    }

    // --- Process creation ---
    PROCESS_INFORMATION piProcInfo{};
    STARTUPINFO siStartInfo{};
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = hChildStdOutWrite;
    siStartInfo.hStdOutput = hChildStdOutWrite;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    if (!CreateProcess(NULL, command.data(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &siStartInfo, &piProcInfo)) {
        strDecodeMsg = "CreateProcess failed.";
        CloseHandle(hChildStdOutRead);
        CloseHandle(hChildStdOutWrite);
        return symbols;
    }

    // --- Close parent's write handle to the pipe ---
    CloseHandle(hChildStdOutWrite);

    // --- Read and process output from child process ---
    DWORD bytesRead = 0;
    char buffer[1024]{};
    std::string output;

    while (ReadFile(hChildStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        output += buffer;
    }

    // --- Final cleanup after reading is complete ---
    CloseHandle(hChildStdOutRead);
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);

    // --- Output parsing and validation ---
    // Simplified parsing logic
    std::istringstream iss(output);
    std::string line;
    int lineCount = 0;
    std::string decodedMessage;

    while (std::getline(iss, line, '\n')) {
        lineCount++;
		std::cout << "Line " << lineCount << ": " << line << std::endl;
        // Use a more robust check for decoded message and symbols line
        if (lineCount == 3) {
            if (line.length() >= 42) {
                decodedMessage = line.substr(42, 40);
            }
            else {
                strDecodeMsg = "Encode error L3 - short line.";
                return TxNoSymbols;
            }
        }
        if ((WSJTX_TXmodeStr == "FT8" && lineCount == 16) || (WSJTX_TXmodeStr == "FT4" && lineCount == 22)) {
            symbols = line;
            break;
        }
    }

    // --- Final symbol processing ---
    if (!symbols.empty()) {
        symbols.erase(std::remove_if(symbols.begin(), symbols.end(), ::isspace), symbols.end());
        if (symbols.length() == requiredLength) {
            strDecodeMsg = removeTrailingSpaces(decodedMessage);
            std::cout << get_millis() << "Encode Done: " << "symbols length = " << symbols.length() << "  strDecodeMsg = " << strDecodeMsg << std::endl;
            std::cout << symbols << std::endl;
            return symbols;
        }
    }

    strDecodeMsg = "ftxcode.exe symbols error - unexpected output.";
    return TxNoSymbols;
}

std::string strPacketType(int type)
{
    switch (type) {
    case 0: return "Heartbeat";
    case 1: return "Status";
    case 2: return "Decode";
    case 3: return "Erase";
    case 4: return "Reply";
    case 5: return "Logged";
	case 6: return "Close";
	case 7: return "HeartbeatEx";
	case 8: return "StatusEx";
	case 9: return "DecodeEx";
	case 10: return "WSPRDecode";
	case 17: return "SetFreq";
    default: return "Unknown";
    }
}

// Function to convert char* to LPWSTR
LPWSTR ConvertToLPWSTR(const char* charString) {
    int len = MultiByteToWideChar(CP_ACP, 0, charString, -1, NULL, 0);
    LPWSTR wcharString = new WCHAR[len];
    MultiByteToWideChar(CP_ACP, 0, charString, -1, wcharString, len);
    return wcharString;
}
