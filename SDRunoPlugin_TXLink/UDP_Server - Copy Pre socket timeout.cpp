#include <iostream>
#include <winsock2.h>
#include <string>
#include <vector>
#include <cstring>
#include <format>
#include <tuple>
#include <chrono>
#include <thread>
#include <windows.h>
#include "WSJTX_Packet.h"
#include "UDP_Server.h"

#define debugUDP

using namespace std;

#pragma comment(lib,"ws2_32.lib") // Winsock Library
#pragma warning(disable:4996) 

//UDP port defaults for wsjtx
#define BUFLEN 512
#define PORT 2237
#define SERVER "127.0.0.1"  // or "localhost" - ip address of UDP server

//externals that provide the simple comms to the plugin
bool EnableServer = false;
bool UDPExitRequest = false;
bool UDPevent = false;          //set to indicate new status update
bool UDPHaltTXevent = false;    // force immediate TX halt
bool TXflg = false;
int TXDF = 0;
std::string TXmode = "FT8";
std::string LastTXmsg = ""; 
bool TXmodeChange = true;
std::string DXcall = "";
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


//wsjtx UDP datagram server with parser and ft8 encoding using wsjtx ft8code.exe utility
//this server is invoked into a seperate thread from the TXlink plugin for SDRuno app.
// status update to the plugin is via the simple extern data values above.
void UDP_Server()
{
    while (!EnableServer) {}            //wait for Ui starup and defaults load from SDRuno ini

    sockaddr_in server{}, client{};

    // initialise winsock
    WSADATA wsa;
    printf("Initialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code: %d", WSAGetLastError());
        exit(0);
    }
    printf("Initialised.\n");

    // create a socket
    SOCKET server_socket;
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create socket: %d", WSAGetLastError());
    }
    printf("Socket created.\n");

    // prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER);
    server.sin_port = htons(PORT);

    // bind
    if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("Bind failed with error code: %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    puts("Bind done.");

    while (!UDPExitRequest)
    {
#ifdef debugUDP
        printf("\nWaiting for data...\n");
#endif
        fflush(stdout);
        char message[BUFLEN] = {};

        // try to receive some data, this is a blocking call
        int message_len;
        int slen = sizeof(sockaddr_in);
        if ((message_len = recvfrom(server_socket, message, BUFLEN, 0, (sockaddr*)&client, &slen)) == SOCKET_ERROR)
        {
            printf("recvfrom() failed with error code: %d", WSAGetLastError());
            exit(0);
        }

        // print details of the client/peer and the data received
        #ifdef debugUDP
            printf("Received packet from %s:%d Len:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), message_len);
        #endif

         ProcessPacket(message, message_len);
    }

    closesocket(server_socket);
    WSACleanup();
}

void ProcessPacket(char * message, int message_len)
{
    std::vector<uint8_t> pkt(message, message + message_len);
    size_t idx = 0;
    
    WSJTX_Packet NewPacket( pkt, idx);
    NewPacket.Decode();
#ifdef debugUDP 
    printf("Type: %d\n", NewPacket.PacketType);
#endif

    if (NewPacket.PacketType == 0) {
        WSJTX_Heartbeat HeartbeatPacket(pkt, NewPacket.index);
        HeartbeatPacket.Decode();
        // emit UI update signal for heartbeat if needed
    }
    else if (NewPacket.PacketType == 1) {
        //status packet contains next ft8 message and TX on/off updates
        WSJTX_Status StatusPacket( pkt, NewPacket.index);
        StatusPacket.Decode();
        std::string msg = StatusPacket.DefaultTXMessage;
        msg = removeTrailingSpaces(msg);
		if (msg == "") { msg = "CQ G4AHN IO91"; }                           //default message if empty

        m_lock2.lock();
        if (StatusPacket.Frequency != curFrequency && newFrequency == 0)
        {
            newFrequency = StatusPacket.Frequency;
            curFrequency = newFrequency;                //reset to zero in form when UDPevent detected
        }
        if (TXmode != StatusPacket.Mode) { TXmodeChange = true; }
        TXmode = StatusPacket.Mode;
        if (TXflg && !StatusPacket.Transmitting && !StatusPacket.TxEnabled) { UDPHaltTXevent = true; }
        TXflg = StatusPacket.Transmitting;  //transmit on/off
        TXDF = StatusPacket.TxDF;           //transmit freq offset added to set TX freq
        if (LastTXmsg == "" || LastTXmsg != msg) { TXsymbols = EncodeMessage(TXmode, msg); }     //list of tones to transmit
		LastTXmsg = msg;                                                                        //store last message sent   
        if (StatusPacket.DXCall != DXcall) { newDXcall = StatusPacket.DXCall; }
        DXcall = StatusPacket.DXCall;
		strReport = StatusPacket.Report;    //report to send
		RxDF = StatusPacket.RxDF;       //receive freq offset
        UDPevent = true;                    //flag for main UDP server code to update Arduino
        m_lock2.unlock();
        // emit UI update signal for status
    }
    else if (NewPacket.PacketType == 2) {
        WSJTX_Decode DecodePacket(pkt, NewPacket.index);
        DecodePacket.Decode();
        // Process DecodePacket and emit UI update signal
    }
    else if (NewPacket.PacketType == 3) {
        WSJTX_Erase ErasePacket(pkt, NewPacket.index);
        ErasePacket.Decode();
        // emit UI erase message signal
    }
    else if (NewPacket.PacketType == 4) {
        WSJTX_Reply ReplyPacket(pkt, NewPacket.index);
        ReplyPacket.Decode();
        // Process ReplyPacket and emit UI update signal
    }
    else if (NewPacket.PacketType == 5) {
        WSJTX_Logged LoggedPacket(pkt, NewPacket.index);
        LoggedPacket.Decode();
        // emit UI update signal for logged packet
    }
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



std::string EncodeMessage(std::string TXmode, std::string strMessage)
{
    // use wsjtx ft8code.exe utility to encode next ft8 message into 79 symbols list (tone numbers)
    // input is the required FT8 message, output is the symbols list (79 chars)
    // the external strDecodeMsg is set to the text message that will be decoded by wsjtx
    // if an error occurs then the symbols are set to "NOCALL" and the strDecodeMsg set to error msg
    // Create a command line string to execute the Consul app

    TxNoSymbols = FT8TxNoSymbols;
    if (TXmode == "FT4") { TxNoSymbols = FT4TxNoSymbols; }      //TX fixed freq.
    
    //TUNE
    if (strMessage == "TUNE")
    {
        strDecodeMsg = "TUNE";
        return TxNoSymbols;                                         //default is FT8
    }

    std::string command = "\"" + strFT8WSJTXpath + "\"";        //default is FT8
    if (TXmode == "FT4") { command = "\"" + strFT4WSJTXpath + "\""; }
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
        if (TXmode == "FT8" && lineCount >= 16) { break; }     // 16th line is the encoded tone list
        if (TXmode == "FT4" && lineCount >= 22) { break; }     // 22th line is the encoded tone list
    }

    //remove spaces and CRLF
    symbols.erase(std::remove(symbols.begin(), symbols.end(), ' '), symbols.end());
    //check and remove cr/lf
    if (TXmode == "FT8" && symbols.length() == 81)
    {
        symbols = symbols.substr(0, 79);
    }
    else if (TXmode == "FT4" && symbols.length() == 107)
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
    std::cout << "\nDone " << symbols.length() << "\n\n";
    std::cout << symbols;
#endif
    return symbols;
}


// Function to convert char* to LPWSTR
LPWSTR ConvertToLPWSTR(const char* charString) {
    int len = MultiByteToWideChar(CP_ACP, 0, charString, -1, NULL, 0);
    LPWSTR wcharString = new WCHAR[len];
    MultiByteToWideChar(CP_ACP, 0, charString, -1, wcharString, len);
    return wcharString;
}
