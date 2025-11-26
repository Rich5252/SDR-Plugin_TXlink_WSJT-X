#pragma once

#include <string>
#include <windows.h>
#include <iostream>

HWND FindTargetWindow(const std::wstring& targetTitle, bool matchExact);
