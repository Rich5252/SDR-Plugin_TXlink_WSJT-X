#include <string>
#include <windows.h>

#include <windows.h>
#include <string>
#include <cstring> // For strstr

#include "FindWindowTitleRoot.h"


//thread safe version of FindWindow by title root

// Structure to pass data safely (thread-safe)
struct EnumData {
    const std::wstring* targetTitle;
    HWND foundHwnd = NULL;
    bool matchExact = false; // New flag to control search mode
};

// Wide-character callback function
BOOL CALLBACK EnumWindowsProcW(HWND hwnd, LPARAM lParam)
{
    EnumData* data = reinterpret_cast<EnumData*>(lParam);

    wchar_t title[256];
    // Use the wide-character API
    GetWindowTextW(hwnd, title, sizeof(title) / sizeof(title[0]));

    std::wstring currentTitle(title);

    bool matchFound = false;

    if (data->matchExact) {
        // Option 1: Exact Match (Title must be IDENTICAL to targetTitle)
        if (currentTitle == *data->targetTitle) {
            matchFound = true;
        }
    }
    else {
        // Option 2: Root/Prefix Match (Title must START WITH targetTitle)
        // std::wstring::find returns 0 if the string is found at the very beginning.
        if (currentTitle.find(*data->targetTitle) == 0) {
            matchFound = true;
        }
    }

    if (matchFound) {
        data->foundHwnd = hwnd;
        return FALSE; // Found it, stop enumeration
    }

    return TRUE; // Continue
}

// DEFINITION: Implements the new parameter
HWND FindTargetWindow(const std::wstring& targetTitle, bool matchExact)
{
    EnumData data;
    data.targetTitle = &targetTitle;
    data.matchExact = matchExact; // Store the search mode

    // Pass the address of the data structure (thread-safe)
    EnumWindows(EnumWindowsProcW, reinterpret_cast<LPARAM>(&data));

    return data.foundHwnd;
}