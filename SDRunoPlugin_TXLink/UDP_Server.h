//#pragma once
#include <mutex>

void ProcessPacket(char message[], int message_len);
void UDP_Server();
LPWSTR ConvertToLPWSTR(const char* charString);
std::string EncodeMessage(std::string strMessage);
std::string removeTrailingSpaces(std::string str);

//following are simple comms with the plugin

extern bool EnableServer;
extern bool UDPExitRequest;

extern bool UDPevent;				//new status update
extern bool TXflg;
extern int TXDF;
extern std::string TXsymbols;
extern std::string strDecodeMsg;
extern std::string strWSJTXpath;
extern std::string strCallsign;
extern std::string strLocator;
