#include <iostream>
#include <thread> // Added for std::this_thread::sleep_for
#include <chrono> // Added for std::chrono::milliseconds
#include <stdexcept>
#include <vector> // Add missing include for std::vector
#include <codecvt>

#include "NamedPipeServer.h"
#include "MessageBox.h"

// --- Class Method Implementations (Partial, assuming remaining methods are defined) ---

// NOTE: The full implementations of NamedPipeServer methods are kept here 
// for self-containment, but only createAndListen is shown in detail for clarity.

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
    std::cout << get_millis() << "Creating pipe: " << pipeName_ << " with connection timeout: ";
    if (timeoutMs == NMPWAIT_WAIT_FOREVER) {
        std::cout << "Infinite" << std::endl;
    }
    else {
        std::cout << timeoutMs << "ms" << std::endl;
    }

    // Create an event handle for overlapped operations
    HANDLE hConnectEvent = CreateEvent(
        NULL,    // Default security attributes
        TRUE,    // Manual reset event
        FALSE,   // Initial state is non-signaled
        NULL);   // No name

    if (hConnectEvent == NULL) {
        std::cerr << get_millis() << "ERROR: Failed to create connection event. Code: " << GetLastError() << std::endl;
        return false;
    }

    OVERLAPPED overlap = { 0 };
    overlap.hEvent = hConnectEvent;

    // 1. Create the Named Pipe
    // We use FILE_FLAG_OVERLAPPED instead of PIPE_WAIT to enable asynchronous I/O.
    hPipe_ = CreateNamedPipeA(
        pipeName_.c_str(),            // Pipe name
        PIPE_ACCESS_DUPLEX |          // Read and write access
        FILE_FLAG_OVERLAPPED,         // *** USE OVERLAPPED I/O ***
        PIPE_TYPE_MESSAGE |           // Message type pipe
        PIPE_READMODE_MESSAGE,        // Message-read mode
        PIPE_UNLIMITED_INSTANCES,     // Max instances
        bufferSize_,                  // Output buffer size
        bufferSize_,                  // Input buffer size
        0,                            // Default timeout is irrelevant with overlapped I/O
        NULL);                        // Default security attributes

    if (hPipe_ == INVALID_HANDLE_VALUE) {
        std::cerr << get_millis() << "ERROR: Failed to create pipe. Code: " << GetLastError() << std::endl;
        CloseHandle(hConnectEvent);
        return false;
    }

    std::cout << get_millis() << "Pipe created. Waiting for VB6 client connection..." << std::endl;

    // 2. Start the asynchronous connection attempt
    // This call is guaranteed to return immediately.
    BOOL fConnected = ConnectNamedPipe(hPipe_, &overlap);

    if (fConnected) {
        // This case should not happen for a newly created pipe using overlapped, 
        // but if it does, it means a client connected instantly.
        std::cout << get_millis() << "SUCCESS: VB6 Client Connected instantly." << std::endl;
        connected_ = true;
        CloseHandle(hConnectEvent);
        return true;
    }

    DWORD dwError = GetLastError();
    if (dwError == ERROR_IO_PENDING) {
        // Connection is pending. Now we wait with the timeout.

        // 3. Wait for the connection event
        DWORD dwWaitResult = WaitForSingleObject(hConnectEvent, timeoutMs);

        if (dwWaitResult == WAIT_OBJECT_0) {
            // The connection succeeded!
            DWORD cbTransfer;
            // GetOverlappedResult confirms the I/O operation (connection) completed.
            if (GetOverlappedResult(hPipe_, &overlap, &cbTransfer, FALSE)) {
                connected_ = true;
                std::cout << get_millis() << "SUCCESS: VB6 Client Connected." << std::endl;
                CloseHandle(hConnectEvent);
                return true;
            }
            else {
                std::cerr << get_millis() << "ERROR: GetOverlappedResult failed after event signaled. Code: " << GetLastError() << std::endl;
            }
        }
        else if (dwWaitResult == WAIT_TIMEOUT) {
            // The timeout expired.
            std::cerr << get_millis() << "ERROR: Client connection timed out (" << timeoutMs << "ms)." << std::endl;
            // Cancel the pending connection and clean up.
            CancelIo(hPipe_);
        }
        else {
            // Other wait error.
            std::cerr << get_millis() << "ERROR: WaitForSingleObject failed. Code: " << dwWaitResult << std::endl;
        }

    }
    else if (dwError != ERROR_PIPE_CONNECTED) {
        // Fatal error during ConnectNamedPipe
        std::cerr << get_millis() << "ERROR: ConnectNamedPipe failed immediately. Code: " << dwError << std::endl;
    }

    // Cleanup on failure/timeout
    CloseHandle(hConnectEvent);
    CloseHandle(hPipe_);
    hPipe_ = INVALID_HANDLE_VALUE;
    connected_ = false;
    return false;
}

/**
 * @brief Reads a single message from the connected client with an optional timeout.
 * @param timeoutMs Timeout in milliseconds. Use 0 for no timeout (infinite wait).
 * @return The received message as a std::string. Empty string on timeout.
 * @throws std::runtime_error on failure or disconnection.
 */
std::string NamedPipeServer::readMessage(DWORD timeoutMs) {
    if (!connected_ || hPipe_ == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Pipe is not connected or handle is invalid.");
    }

    // Use a dedicated event handle for the read operation
    HANDLE hReadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hReadEvent == NULL) {
        throw std::runtime_error("Failed to create read event.");
    }

    OVERLAPPED overlap = { 0 };
    overlap.hEvent = hReadEvent;

    // Use a vector to act as the byte buffer
    std::vector<char> buffer(bufferSize_);
    DWORD cbRead = 0;

    // 1. Start the asynchronous read operation
    // ReadFile will immediately return false with ERROR_IO_PENDING because of FILE_FLAG_OVERLAPPED.
    BOOL fSuccess = ReadFile(
        hPipe_,
        buffer.data(),
        bufferSize_ - 1, // Read up to bufferSize-1 to ensure room for null terminator
        &cbRead,
        &overlap); // Pass OVERLAPPED structure

    // 2. Check read status
    if (!fSuccess) {
        DWORD dwError = GetLastError();
        if (dwError == ERROR_IO_PENDING) {
            // Read is pending. Wait for the result with the specified timeout.
            DWORD dwWaitResult = WaitForSingleObject(hReadEvent, timeoutMs);

            if (dwWaitResult == WAIT_OBJECT_0) {
                // Read operation completed within the timeout.
                // GetOverlappedResult confirms the read and gets the byte count.
                if (!GetOverlappedResult(hPipe_, &overlap, &cbRead, FALSE)) {
                    // Check specifically for broken pipe during result check
                    if (GetLastError() == ERROR_BROKEN_PIPE) {
                        connected_ = false;
                        CloseHandle(hReadEvent);
                        throw std::runtime_error("Client disconnected (Broken Pipe).");
                    }
                    CloseHandle(hReadEvent);
                    throw std::runtime_error("GetOverlappedResult failed during read. Code: " + std::to_string(GetLastError()));
                }
            }
            else if (dwWaitResult == WAIT_TIMEOUT) {
                // Timeout occurred. Cancel the pending read operation.
                CancelIo(hPipe_);
                CloseHandle(hReadEvent);
                return ""; // Return empty string on timeout
            }
            else {
                // WaitForSingleObject failed.
                CloseHandle(hReadEvent);
                throw std::runtime_error("WaitForSingleObject failed during read wait. Code: " + std::to_string(dwWaitResult));
            }
        }
        else if (dwError == ERROR_BROKEN_PIPE) {
            connected_ = false;
            CloseHandle(hReadEvent);
            throw std::runtime_error("Client disconnected (Broken Pipe).");
        }
        else {
            // Other ReadFile error
            CloseHandle(hReadEvent);
            throw std::runtime_error("Error initiating read from pipe. Code: " + std::to_string(dwError));
        }
    }
    // If fSuccess was true here, it means the read completed instantly (rare for a pipe read).

    // 3. Process successful read

    // Copy the exact number of bytes read into a std::string, filtering out null bytes.
    std::string receivedData;
    receivedData.reserve(cbRead);

    // *** FIX: Filter out null bytes (0x00) to clean up wide-character streams ***
    for (DWORD i = 0; i < cbRead; ++i) {
        if (buffer[i] != '\0') {
            receivedData += buffer[i];
        }
    }

    CloseHandle(hReadEvent);
    return receivedData;
}

bool NamedPipeServer::writeMessage(const std::string& message) {
    if (!connected_ || hPipe_ == INVALID_HANDLE_VALUE) {
        std::cerr << get_millis() << "WARNING: Cannot write, pipe is not connected." << std::endl;
        return false;
    }

    // --- 1. CONVERT 8-bit std::string (UTF-8/ASCII) to 16-bit std::wstring (UTF-16/Unicode) ---
    // Use the standard C++ converter for wide strings.
    // NOTE: std::codecvt_utf8 is deprecated in C++17, but is widely used in older projects.
#if __cplusplus < 201703L
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wdataToWrite = converter.from_bytes(message);
#else
// For C++17 and later, this conversion requires more modern (or OS-specific) methods.
// For simplicity and commonality in older engines, we assume pre-C++17 or platform conversion utilities.
    std::wstring wdataToWrite(message.begin(), message.end()); // Simple ASCII/char to wchar_t conversion
#endif

    // 2. Append a WIDE null terminator (L'\0') for robustness on the client side.
    wdataToWrite += L'\0';

    // 3. Calculate the number of BYTES to write. Each wchar_t is 2 bytes.
    DWORD cbToWrite = wdataToWrite.length() * sizeof(wchar_t);
    DWORD cbWritten = 0;

    // --- 4. Write the 16-bit data ---
    BOOL fSuccess = WriteFile(
        hPipe_,
        wdataToWrite.c_str(), // Use c_str() to get the wide character pointer (wchar_t*)
        cbToWrite,            // Write the total number of bytes
        &cbWritten,
        NULL);

    if (!fSuccess) {
        std::cerr << get_millis() << "ERROR: Failed to write to pipe. Code: " << GetLastError() << std::endl;
        return false;
    }

    // Check if the expected number of bytes were written
    if (cbWritten != cbToWrite) {
        std::cerr << get_millis() << "WARNING: Only wrote " << cbWritten << " of " << cbToWrite << " bytes." << std::endl;
    }

    return true;
}

/**
 * @brief Writes a message to the connected client.
 * @param message The message to send.
 * @return true if successful, false otherwise.
 */
/*
bool NamedPipeServer::writeMessage(const std::string& message) {
    if (!connected_ || hPipe_ == INVALID_HANDLE_VALUE) {
        std::cerr << "WARNING: Cannot write, pipe is not connected." << std::endl;
        return false;
    }

    DWORD cbWritten = 0;

    // Write the message plus a null terminator to be robust on the client side
    std::string dataToWrite = message + '\0';

    // NOTE: WriteFile is synchronous and typically does not require overlapped I/O 
    // unless the write is expected to be very slow (which is rare for pipes).
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
*/

/**
 * @brief Disconnects and closes the pipe handle.
 */
/*
void NamedPipeServer::close() {
    if (hPipe_ != INVALID_HANDLE_VALUE) {
        if (connected_) {
            FlushFileBuffers(hPipe_);
            DisconnectNamedPipe(hPipe_);
            connected_ = false;
            std::cout << "NamedPipeServer::close - Pipe disconnected." << std::endl;
        }
        std::cout << "NamedPipeServer::close - Close handle." << std::endl;
        CloseHandle(hPipe_);
        hPipe_ = INVALID_HANDLE_VALUE;
    }
}
*/

void NamedPipeServer::close() {
    if (hPipe_ != INVALID_HANDLE_VALUE) {

        // 1. Swap the handle out early to prevent race conditions and re-use.
        // Use a temporary handle for all operations.
        HANDLE tempPipeHandle = hPipe_;
        hPipe_ = INVALID_HANDLE_VALUE;

        // 2. CANCEL ALL PENDING I/O
        // CancelIoEx is preferred over CancelIo for modern Windows.
        // Passing NULL cancels ALL pending I/O operations issued by the calling thread.
        // It is CRUCIAL that the thread blocked on I/O is the one that started it.
        // If the close() function is called from a DIFFERENT thread than the one
        // blocked on ConnectNamedPipe/ReadFile, you MUST use CancelIoEx.
        if (CancelIoEx(tempPipeHandle, NULL) == 0) {
            // In a client crash scenario, CancelIoEx might fail if no I/O is pending.
            // We can ignore the error if it's NOT a critical error (like ERROR_INVALID_HANDLE).
            // std::cerr << "CancelIoEx failed (may be harmless), error: " << GetLastError() << std::endl;
        }

        // 3. CLEANUP AND DISCONNECT (Optional but good practice)
        if (connected_) {
            // Flush is often ineffective if the client is crashed, but we try.
            FlushFileBuffers(tempPipeHandle);

            // This attempts a clean shutdown but is less reliable than CloseHandle.
            // It will unblock a waiting client, but if the client crashed, it may briefly hang.
            DisconnectNamedPipe(tempPipeHandle);
            connected_ = false;
            // std::cout << "NamedPipeServer::close - Pipe disconnected." << std::endl;
        }

        // 4. FORCIBLY CLOSE THE HANDLE
        // This is the definitive action. It forces the end of all I/O and terminates
        // any thread blocked on this specific handle (tempPipeHandle).
        std::cout << get_millis() << "NamedPipeServer::close - Handle closed." << std::endl;
        CloseHandle(tempPipeHandle);
    }
}

/**
 * @brief Checks if the pipe is currently connected to a client.
 */
bool NamedPipeServer::isConnected() const {
    return connected_;
}

/*
// --- Demonstration Main Function (This would be your TXLink pipe thread function) ---
int main() {
    const std::string PIPE_NAME = "\\\\.\\pipe\\TXLinkCommandPipe";
    NamedPipeServer server(PIPE_NAME);

    // --- 5-Second Connection Timeout ---
    const DWORD CONNECTION_TIMEOUT_MS = 5000;
    // --- 2-Second Wait Between Failed Attempts ---
    const int RETRY_WAIT_MS = 2000;

    // Flag to control the outer connection loop
    bool isConnected = false;

    // 1. Connection/Reconnection Loop
    std::cout << "Starting connection monitoring thread..." << std::endl;
    do {
        // Attempt to create the pipe and connect with a timeout
        if (server.createAndListen(CONNECTION_TIMEOUT_MS)) {
            isConnected = true;
            break; // Exit connection loop, proceed to message loop
        }
        else {
            // Connection failed (timed out or error).
            std::cout << "VB6 client not found. Retrying in " << RETRY_WAIT_MS / 1000 << " seconds..." << std::endl;
            // IMPORTANT: Wait before retrying to prevent CPU usage spike.
            std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_WAIT_MS));
        }

    } while (true); // Loop until connection is established

    // 2. Main Message Loop (Only runs when connected)
    std::cout << "\nSUCCESS: Connection established. Starting non-blocking read loop (100ms timeout)..." << std::endl;
    while (server.isConnected()) {
        try {
            // Read with a 100 millisecond timeout
            std::string message = server.readMessage(100);

            if (!message.empty()) {
                std::cout << "\n[APPLICATION] Received Command: " << message << std::endl;
                if (message == "GET_STATUS") {
                    server.writeMessage("STATUS:READY");
                }
            }

        }
        catch (const std::runtime_error& e) {
            // Handle disconnection (e.g., if VB6 app closes)
            std::cerr << "Pipe Error: " << e.what() << std::endl;
            server.close();
            isConnected = false;

            // Break the read loop to return to the connection loop
            break;
        }
    }

    // If the loop exits due to disconnection, the TXLink thread can either:
    // a) Restart the entire process by jumping back to the connection loop.
    // b) Terminate, as shown here.

    std::cout << "Server loop finished. Press Enter to exit." << std::endl;
    std::cin.get();
    return 0;
}
*/
