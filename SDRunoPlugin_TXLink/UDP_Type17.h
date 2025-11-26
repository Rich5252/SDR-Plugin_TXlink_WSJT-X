#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "UDP_Server.h"


bool sendSetFreqCommand(
    SOCKET tx_socket, // Pass your existing server_socket here
    const std::string& sessionId,
    uint64_t dialFrequencyHz,
    bool txEnabled,
    const std::string& mode,
    uint32_t freqTolerance,
    const std::string& subMode,
    bool fastMode,
    uint32_t trPeriod,
    uint32_t rxDF,
    uint32_t txDF,
    const std::string& dxCall,
    const std::string& dxGrid,
    bool generateMessages,
    bool txEven
);

void printHexFrequency(uint64_t frequency);

void hexDumpBuffer(const std::vector<char>& buffer);