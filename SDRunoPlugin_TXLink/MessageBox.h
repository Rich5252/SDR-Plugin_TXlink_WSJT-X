#pragma once
#include <windows.h>
#include <iostream>
#include <sstream>
#include <string>

void LogAndDisplayError(const std::string& errorMessage);

long long ll_get_millis();
std::string get_millis();

extern int SDRthreadCount;