#include <iostream>
#include "NamedPipeServer.h"


// --- Class Method Implementations ---

NamedPipeServer::NamedPipeServer(const std::string& pipeName, size_t bufferSize)
    : pipeName_(pipeName), bufferSize_(bufferSize), hPipe_(INVALID_HANDLE_VALUE), connected_(false) {
}

NamedPipeServer::~NamedPipeServer() {
    close();
}

/**
 * @brief Creates the pipe and blocks until a client connects, or until the timeout elapses.
 * @param timeoutMs Timeout in milliseconds for the connection attempt. Defaults to infinite wait.
 * @return true if successful, false otherwise.
 */
bool NamedPipeServer::createAndListen(DWORD timeoutMs) {
    std::cout << "Creating pipe: " << pipeName_ << " with connection timeout: ";
    if (timeoutMs == NMPWAIT_WAIT_FOREVER) {
        std::cout << "Infinite" << std::endl;
    }
    else {
        std::cout << timeoutMs << "ms" << std::endl;
    }

    // 1. Create the Named Pipe
    // The last parameter (timeoutMs) is the default time-out value for ConnectNamedPipe.
    hPipe_ = CreateNamedPipeA(
        pipeName_.c_str(),            // Pipe name
        PIPE_ACCESS_DUPLEX,           // Read and write access (allows two-way comms)
        PIPE_TYPE_MESSAGE |           // Message type pipe
        PIPE_READMODE_MESSAGE |       // Message-read mode
        PIPE_WAIT,                    // Blocking mode
        PIPE_UNLIMITED_INSTANCES,     // Max instances
        bufferSize_,                  // Output buffer size
        bufferSize_,                  // Input buffer size
        timeoutMs,                    // *** CONNECTION TIMEOUT ***
        NULL);                        // Default security attributes

    if (hPipe_ == INVALID_HANDLE_VALUE) {
        std::cerr << "ERROR: Failed to create pipe. Code: " << GetLastError() << std::endl;
        return false;
    }

    std::cout << "Pipe created. Waiting for VB6 client connection..." << std::endl;

    // 2. Wait for the client to connect (this call respects the timeout set above)
    BOOL fConnected = ConnectNamedPipe(hPipe_, NULL);

    if (!fConnected) {
        DWORD error = GetLastError();

        if (error == ERROR_PIPE_LISTENING) {
            // This error (535) means the pipe is still listening, but the connection attempt timed out.
            std::cerr << "ERROR: Client connection timed out (" << timeoutMs << "ms)." << std::endl;
        }
        else {
            // Other fatal errors
            std::cerr << "ERROR: Failed to connect or handle closed. Code: " << error << std::endl;
        }

        CloseHandle(hPipe_);
        hPipe_ = INVALID_HANDLE_VALUE;
        connected_ = false;
        return false;
    }

    connected_ = true;
    std::cout << "SUCCESS: VB6 Client Connected." << std::endl;
    return true;
}

/**
 * @brief Reads a single message from the connected client with an optional timeout.
 * @param timeoutMs Timeout in milliseconds. Use 0 for default pipe wait (50ms default).
 * Use NMPWAIT_WAIT_FOREVER (0xFFFFFFFF) for infinite blocking.
 * @return The received message as a std::string. Empty string on timeout.
 * @throws std::runtime_error on failure or disconnection.
 */
std::string NamedPipeServer::readMessage(DWORD timeoutMs) {
    if (!connected_ || hPipe_ == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Pipe is not connected or handle is invalid.");
    }

    DWORD dwMode = PIPE_READMODE_MESSAGE;
    DWORD dwTimeout = timeoutMs;

    // 1. Set the pipe state to apply the requested timeout
    // Check if timeoutMs is not 0. If it is 0, the pipe uses its creation-time default (50ms).
    // We only call SetNamedPipeHandleState if the user passed a specific timeout.
    if (dwTimeout != 0 && dwTimeout != NMPWAIT_WAIT_FOREVER) {
        // Dynamically set the read timeout before calling ReadFile.
        BOOL fSuccessState = SetNamedPipeHandleState(
            hPipe_,
            &dwMode,           // We keep message read mode
            NULL,              // Don't change max instances/collect count
            &dwTimeout);       // Set the read timeout in milliseconds

        if (!fSuccessState) {
            // If setting the state fails, log the issue and fall back to existing read mode.
            std::cerr << "WARNING: Failed to set pipe read timeout state. Code: " << GetLastError() << std::endl;
        }
    }


    // Use a vector to act as the byte buffer
    std::vector<char> buffer(bufferSize_);
    DWORD cbRead = 0;

    // 2. ReadFile is now governed by the timeout set above (or default blocking)
    BOOL fSuccess = ReadFile(
        hPipe_,
        buffer.data(),
        bufferSize_ - 1, // Read up to bufferSize-1 to ensure room for null terminator
        &cbRead,
        NULL);

    // 3. Check read results
    if (!fSuccess) {
        DWORD error = GetLastError();

        if (error == ERROR_BROKEN_PIPE) {
            // Client disconnected gracefully
            connected_ = false;
            throw std::runtime_error("Client disconnected (Broken Pipe).");
        }
        else if (error == ERROR_SEM_TIMEOUT) {
            // Read timed out (ERROR_SEM_TIMEOUT = 121)
            // This means no message was available within the specified timeout period.
            return ""; // Return empty string on timeout
        }
        else {
            // Other read error
            throw std::runtime_error("Error reading from pipe. Code: " + std::to_string(error));
        }
    }

    // If successful:

    // CORRECT FIX: Copy the exact number of bytes read into a std::string.
    std::string receivedData;
    receivedData.assign(buffer.data(), cbRead);

    // Remove the trailing null-terminator added by the VB6 client code (Chr$(0))
    if (!receivedData.empty() && receivedData.back() == '\0') {
        receivedData.pop_back();
    }

    return receivedData;
}

/**
 * @brief Writes a message to the connected client.
 * @param message The message to send.
 * @return true if successful, false otherwise.
 */
bool NamedPipeServer::writeMessage(const std::string& message) {
    if (!connected_ || hPipe_ == INVALID_HANDLE_VALUE) {
        std::cerr << "WARNING: Cannot write, pipe is not connected." << std::endl;
        return false;
    }

    DWORD cbWritten = 0;

    // Write the message plus a null terminator to be robust on the client side
    std::string dataToWrite = message + '\0';

    BOOL fSuccess = WriteFile(
        hPipe_,
        dataToWrite.c_str(),
        dataToWrite.length(), // Write the bytes including the null terminator
        &cbWritten,
        NULL);

    if (!fSuccess) {
        std::cerr << "ERROR: Failed to write to pipe. Code: " << GetLastError() << std::endl;
        return false;
    }
    return true;
}

/**
 * @brief Disconnects and closes the pipe handle.
 */
void NamedPipeServer::close() {
    if (hPipe_ != INVALID_HANDLE_VALUE) {
        if (connected_) {
            FlushFileBuffers(hPipe_);
            DisconnectNamedPipe(hPipe_);
            connected_ = false;
            std::cout << "Pipe disconnected." << std::endl;
        }
        CloseHandle(hPipe_);
        hPipe_ = INVALID_HANDLE_VALUE;
    }
}

/**
 * @brief Checks if the pipe is currently connected to a client.
 */
bool NamedPipeServer::isConnected() const {
    return connected_;
}


/*
// --- Demonstration Main Function ---
int mainxxx() {
    const std::string PIPE_NAME = "\\\\.\\pipe\\TXLinkCommandPipe";
    NamedPipeServer server(PIPE_NAME);

    // 1. Create and listen. We are now using a 5000ms (5 second) timeout for connection.
    // If the VB6 client doesn't connect within 5 seconds, this function will return false.
    if (!server.createAndListen(5000)) {
        std::cerr << "Server failed to connect client within 5 seconds. Exiting." << std::endl;
        std::cin.get();
        return 1;
    }

    // 2. Main message loop
    std::cout << "\nStarting non-blocking read loop (100ms timeout)..." << std::endl;
    while (server.isConnected()) {
        try {
            // Read with a 100 millisecond timeout
            std::string message = server.readMessage(100);

            if (!message.empty()) {
                // --- CORE APPLICATION LOGIC HERE ---
                std::cout << "\n[APPLICATION] Received Command: " << message << std::endl;

                // Example: Respond to a command
                if (message == "GET_STATUS") {
                    std::cout << "[APPLICATION] Sending status back..." << std::endl;
                    server.writeMessage("STATUS:READY");
                }
                // Example: Implement RTTY command execution
                else if (message.rfind("SET_PTT:", 0) == 0) {
                    std::cout << "[APPLICATION] Executing RTTY PTT command: " << message << std::endl;
                }
                // --- END CORE APPLICATION LOGIC ---
            }
            // If message is empty, it was a timeout, and the loop continues, allowing other tasks to run.

        }
        catch (const std::runtime_error& e) {
            // Handle disconnection or error
            std::cerr << "Pipe Error: " << e.what() << std::endl;
            server.close();
        }
    }

    std::cout << "Server loop finished. Press Enter to exit." << std::endl;
    std::cin.get();
    return 0;
}
*/