#pragma once

#include <mutex>
#include <iunoplugincontroller.h>
#include "SimpleSerial.h"
#include "MessageBox.h"
#include <atomic>

//This server is started in a separate thread from SDRunoPlugin_TXLink.cpp
class SDRunoPlugin_TXLink;

void Tuner_Server_Loop(IUnoPluginController& _controller, SDRunoPlugin_TXLink & _parent);
extern std::atomic<bool> TunerExitRequest;				//break out of loop and tidy up on app close

LPWSTR ConvertToLPWSTR(const char* charString);
