#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <windows.h>

// Global variable for target window title
//std::wstring targetWindowTitle = L"WSJT-X   v2.7.0   by K1JT et al.";
std::wstring targetWindowTitle = L"WSJT-X   v3.0.0  improved PLUS edition";

// Function to find a window by its title
HWND FindTargetWindow() {
    return FindWindowW(nullptr, targetWindowTitle.c_str());
}

// **IMPLEMENTATION OF SetForegroundWindowEx**
bool SetForegroundWindowEx(HWND hwnd) {
    if (hwnd == nullptr) {
        return false;
    }

    // If the window is minimized, restore it
    if (IsIconic(hwnd)) {
        ShowWindow(hwnd, SW_RESTORE);
    }

    // Try to set the foreground window
    if (SetForegroundWindow(hwnd)) {
        return true;
    }

    // If SetForegroundWindow fails (often due to focus restrictions),
    // try attaching the current thread's input queue to the target window's thread.
    DWORD foreThread = GetGUIThreadInfo(0, nullptr);
    DWORD ourThread = GetCurrentThreadId();
    if (foreThread != ourThread) {
        AttachThreadInput(ourThread, foreThread, TRUE);
        bool result = SetForegroundWindow(hwnd);
		AttachThreadInput(ourThread, foreThread, FALSE);            //detach the input queue
        if (result) {
            return true;
        }
    }

    // As a last resort, try bringing the window to the top without explicitly setting focus
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    return false;
}

// Helper function to create a KEYBDINPUT structure for a key event
INPUT CreateKeyInput(WORD vk, DWORD flags) {
    INPUT input;
    ZeroMemory(&input, sizeof(input));
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    input.ki.wScan = MapVirtualKeyW(vk, 0);
    input.ki.dwFlags = flags;
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
    return input;
}

// Function to send a single key event
void SendKeyEvent(WORD vk, DWORD flags, int delay_ms) {
    INPUT input = CreateKeyInput(vk, flags);
    UINT uSent = SendInput(1, &input, sizeof(INPUT));
    if (uSent != 1) {
        std::cerr << "SendInput failed: " << GetLastError() << std::endl;
    }
    if (delay_ms > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
}

// Function to send a key combination to the target window, sending individual events
void SendKeyCombinationIndividual(HWND hwnd, WORD key1, WORD key2, WORD key3, int delay_ms) {
    if (hwnd == nullptr) {
        std::cerr << "Target window not found." << std::endl;
        return;
    }

    // Bring the target window to the foreground
    if (!SetForegroundWindowEx(hwnd)) {
        std::cerr << "Failed to set the target window to the foreground." << std::endl;
        return;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
   
    SendKeyEvent(key1, 0, delay_ms);            // Key 1 Down (CTRL)  
    SendKeyEvent(key2, 0, delay_ms);            // Key 2 Down (SHIFT)
    SendKeyEvent(key3, 0, delay_ms);            // Key 3 Down (F11)
    SendKeyEvent(key3, KEYEVENTF_KEYUP, delay_ms);  // Key 3 Up (F11)
    SendKeyEvent(key2, KEYEVENTF_KEYUP, delay_ms);  // Key 2 Up (SHIFT)   
    SendKeyEvent(key1, KEYEVENTF_KEYUP, delay_ms);  // Key 1 Up (CTRL)
}



int Send_CtrlShftF11() {
    HWND targetWindow = FindTargetWindow();

    if (targetWindow) {
        // Send CTRL+SHIFT+F11
        SendKeyCombinationIndividual(targetWindow, VK_CONTROL, VK_SHIFT, VK_F11, 100);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));               //allow time to process otherwise it will miss CTRL and move Tx frequency in graph
        std::cout << "F11 sent to WSJT-X" << std::endl;
    }
    else {
        std::cerr << "Target window not found for SendKeys F11." << std::endl;
        return 1;
    }

    return 0;
}

int Send_CtrlShftF12() {
    HWND targetWindow = FindTargetWindow();

    if (targetWindow) {
        SendKeyCombinationIndividual(targetWindow, VK_CONTROL, VK_SHIFT, VK_F12, 20);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "F12 sent to WSJT-X" << std::endl;
    }
    else {
        std::cerr << "Target window not found for SendKeys F12." << std::endl;
        return 1;
    }

    return 0;
}