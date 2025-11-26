
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <windows.h>
#include <stdexcept>

#include "FindWindowTitleRoot.h"


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

HWND Find_WSJTX_Window() {
    HWND target = FindTargetWindow(L"WSJT-X   v2.7.0   by K1JT et al.", true);
    if (!target) {
        target = FindTargetWindow(L"WSJT-X   v3.0.0  improved PLUS edition", true);
    }
    if (!target) {
        target = FindTargetWindow(L"WSJT-X   v3.0.0-rc1 ", true);
    }
    return target;
}

// Function to send a key combination
void SendKeyCombination(HWND hwnd, WORD key1, WORD key2, WORD key3) {
    if (hwnd == nullptr) {
        std::cerr << "Target window not found." << std::endl;
        return;
    }

    INPUT input[6]; // For three keys down and three keys up

    // Key 1 Down (e.g., VK_CONTROL)
    input[0] = CreateKeyInput(key1, 0);

    // Key 2 Down (e.g., VK_SHIFT)
    input[1] = CreateKeyInput(key2, 0);

    // Key 3 Down (e.g., VK_F11)
    input[2] = CreateKeyInput(key3, 0);

    // Key 3 Up
    input[3] = CreateKeyInput(key3, KEYEVENTF_KEYUP);

    // Key 2 Up
    input[4] = CreateKeyInput(key2, KEYEVENTF_KEYUP);

    // Key 1 Up
    input[5] = CreateKeyInput(key1, KEYEVENTF_KEYUP);

    DWORD targetThreadId = GetWindowThreadProcessId(hwnd, nullptr);
    DWORD currentThreadId = GetCurrentThreadId();

    if (AttachThreadInput(currentThreadId, targetThreadId, TRUE)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        UINT uSent = SendInput(ARRAYSIZE(input), input, sizeof(INPUT));
        if (uSent != ARRAYSIZE(input)) {
            std::cerr << "SendInput failed: " << GetLastError() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        AttachThreadInput(currentThreadId, targetThreadId, FALSE);
    }
    else {
        std::cerr << "AttachThreadInput failed: " << GetLastError() << std::endl;
        for (int i = 0; i < ARRAYSIZE(input); ++i) {
			UINT uSent = SendInput(1, &input[i], sizeof(INPUT));
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            if (uSent != 1) {
                std::cerr << "SendInput failed: " << GetLastError() << std::endl;
            }
		}
        /*
        UINT uSent = SendInput(ARRAYSIZE(input), input, sizeof(INPUT));
        if (uSent != ARRAYSIZE(input)) {
            std::cerr << "SendInput failed: " << GetLastError() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        */
    }
}

int Send_CtrlShftF11() {
    try {
        // Replace with the actual title of the window you want to control.
    
        HWND targetWindow = Find_WSJTX_Window();

        if (targetWindow) {
            // Bring the window to the foreground to ensure the keys are sent to it.
            SetForegroundWindow(targetWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Send CTRL+SHIFT+F11
            SendKeyCombination(targetWindow, VK_SHIFT, VK_CONTROL, VK_F11);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        else {
            std::cerr << "Target window not found. Please check the window title." << std::endl;
            // You can print a list of window titles here to help with debugging
            // For example, by using the code I provided previously.
        }

        std::cout << "Tuner: Sent CTRL-SHFT-F11" << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "Tuner SendKey error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

int Send_CtrlShftF12() {
    try {
        // Replace with the actual title of the window you want to control.
        HWND targetWindow = Find_WSJTX_Window();

        if (targetWindow) {
            // Bring the window to the foreground to ensure the keys are sent to it.
            SetForegroundWindow(targetWindow);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

             //To send CTRL+SHIFT+F12, uncomment the line below:
             SendKeyCombination(targetWindow, VK_SHIFT, VK_CONTROL, VK_F12);
        }
        else {
            std::cerr << "Target window not found. Please check the window title." << std::endl;
            // You can print a list of window titles here to help with debugging
            // For example, by using the code I provided previously.
            return 1;
        }

        std::cout << "Tuner: Sent CTRL-SHFT-F12" << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "Tuner: SendKey error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

