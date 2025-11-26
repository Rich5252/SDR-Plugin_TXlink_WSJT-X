#pragma once

#include <string>
#include <atomic>

#include "MessageQueue.h"


//Thread to process messages to DXspider via DXspiderSpotOutQ queue
void DXspiderLoop();                                //thread loop started in SDRunoPlugin_TXLink.cpp
void DXspider_force_close();                        //forcefully close the socket to cancel any pending operations
                                                    //called on app exit to ensure clean shutdown - see TXLink closure code
extern std::atomic<bool> DXspiderExitRequest;		//break out of loop and tidy up on app close


//NOTE defaults are stored in SDRUno ini
struct DXspot_t {                                   //data structure for DXspot messages
    std::string dxSpiderPort;                       //eg "dxspider.co.uk:7300"
    std::string myCall;                             //eg "G4AHN-2"
    std::string DXspot;                             // spot message format: "DX <frequency> <dx_call> <notes>"
};
       
extern ThreadSafeMessageQueue<DXspot_t> DXspiderSpotOutQ;       //use Queue to send spot messages to DXspider thread
    