#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>

// Define the maximum size for the command buffer (matching the VB6 setup)
#define DEFAULT_BUFSIZE 512
// Define the constant for infinite wait (use this for blocking behavior if preferred)
//#define NMPWAIT_WAIT_FOREVER 0xFFFFFFFF           //defined in WinBase.h

/**
 * @brief A class to manage the C++ side of a Named Pipe connection (Server role).
 * * Handles creation, listening, reading, writing, and closing the pipe handle.
 */
class NamedPipeServer {
public:
    /**
     * @brief Constructs the NamedPipeServer.
     * @param pipeName The unique name of the pipe (e.g., "\\\\.\\pipe\\TXLinkCommandPipe").
     * @param bufferSize The maximum size of messages to send/receive.
     */
    NamedPipeServer(const std::string& pipeName, size_t bufferSize = DEFAULT_BUFSIZE);

    /**
     * @brief Destructor ensures the pipe is closed.
     */
    ~NamedPipeServer();

    /**
     * @brief Creates the pipe and blocks until a client connects.
     * @return true if successful, false otherwise.
     */
    bool createAndListen(DWORD timeoutMs = NMPWAIT_WAIT_FOREVER);

    /**
     * @brief Reads a single message from the connected client with an optional timeout.
     * @param timeoutMs Timeout in milliseconds. Use 0 for default pipe wait (50ms default).
     * Use NMPWAIT_WAIT_FOREVER (0xFFFFFFFF) for infinite blocking.
     * @return The received message as a std::string. Empty string on timeout.
     * @throws std::runtime_error on failure or disconnection.
     */
    std::string readMessage(DWORD timeoutMs = 0);

    /**
     * @brief Writes a message to the connected client.
     * @param message The message to send.
     * @return true if successful, false otherwise.
     */
    bool writeMessage(const std::string& message);

    /**
     * @brief Disconnects and closes the pipe handle.
     */
    void close();

    /**
     * @brief Checks if the pipe is currently connected to a client.
     */
    bool isConnected() const;

private:
    std::string pipeName_;
    size_t bufferSize_;
    HANDLE hPipe_;
    bool connected_;
};
