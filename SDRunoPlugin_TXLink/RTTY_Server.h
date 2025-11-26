#pragma once


#include <mutex>
#include <atomic>

#include "MessageBox.h"
#include <iunoplugincontroller.h>
#include "MessageQueue.h"



//This server is started in a separate thread from SDRunoPlugin_TXLink.cpp
class SDRunoPlugin_TXLink;

void RTTY_Server_Loop(IUnoPluginController& _controller, SDRunoPlugin_TXLink& _parent);
extern std::atomic<bool> RTTYExitRequest;			//break out of loop and tidy up on app close
extern ThreadSafeMessageQueue<std::string> RTTYmsgOutQ;
