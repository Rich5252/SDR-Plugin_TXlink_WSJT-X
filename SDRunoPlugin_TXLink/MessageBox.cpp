#include <windows.h>
#include <iostream>
#include <sstream>
#include <string>
#include "MessageBox.h"

int SDRthreadCount = 0;

// Function to display a message box with a given message
void ShowErrorDialog(const std::string& message) {
    MessageBoxA(NULL, message.c_str(), "Error", MB_OK | MB_ICONERROR);
}

// Function to simulate error logging and display the message box
void LogAndDisplayError(const std::string& errorMessage) {
    std::stringstream errorStream;
    errorStream << errorMessage;
    std::string errorString = errorStream.str();

    std::cout << errorString << std::endl; // Log to console
    ShowErrorDialog(errorString); // Show error dialog
}

#include <chrono>

// A helper function to get the current time in milliseconds
long long ll_get_millis() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

// A helper function to get a string with the time elapsed since the first call
std::string get_millis() {
    // This static variable stores the starting time point, initialized only once.
    static auto start_time = std::chrono::steady_clock::now();

    // Calculate the duration from the start time to the current time.
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);

    return std::to_string(elapsed.count()) + ": ";
}
