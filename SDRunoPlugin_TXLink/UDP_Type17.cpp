// --- Put this function in your UDP_Server.cpp or a dedicated helper file ---

#include <iostream>
#include <vector>
#include <string>
#include <algorithm> // for std::reverse (if needed)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iomanip>              // For std::hex, std::setw, std::setfill
#include <intrin.h>
#include "UDP_Type17.h"

// Assuming you have access to WSJTX_Packet.h definitions
#include "WSJTX_Packet.h"

//UDP port defaults for wsjtx
#define BUFLEN 512
#define PORT 2237
#define SERVER "127.0.0.1"  // or "localhost" - ip address of UDP server

#pragma comment(lib, "Ws2_32.lib")

/*
// Raw data for 28.075 MHz FT8, Session ID "WSJT-X"
const char raw_packet_data[89] = {
    // Header (Magic, Schema=2, Type=17)
    (char)0xAD, (char)0xBC, (char)0xCB, (char)0xDA, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x11,

    // 1. Session ID (Length=7, "WSJT-X") - STARTS at offset 12, ends at offset 22
    0x00, 0x00, 0x00, 0x06, 0x57, 0x53, 0x4A, 0x54, 0x2D, 0x59, // 0x57='W', 0x58='X'

    // 2. Dial Freq (28.075 Mhz in mHz: 0x0000000687693080) - STARTS at offset 23
    //0x00, 0x00, 0x00, 0x06, (char)0x87, (char)0x69, 0x30, (char)0x80,
    //24915000
    //0x00, 0x00, 0x00, 0x00, 0x01, 0x7C, 0x2C, 0x38,
    //28075000
    0x00, 0x00, 0x00, 0x00, 0x01, 0xAC, 0x63, 0xF8,
    //28074000
    //0x00, 0x00, 0x00, 0x00, 0x01, 0xAC, 0x60, 0x10,

    // 3. Tx Enabled (false)
    0x00,

    // 4. Mode (Length=3, "FT8")
    0x00, 0x00, 0x00, 0x03, 0x46, 0x54, 0x38,

    // 5. Freq Tolerance (2500)
    0x00, 0x00, 0x09, (char)0xC4,

    // 6. Sub Mode (Length=0, "")
    0x00, 0x00, 0x00, 0x00,

    // 7. Fast Mode (False)
    0x00,

    // 8. Tr Period (15)
    0x00, 0x00, 0x00, 0x0F,

    // 9. Rx DF (1500)
    0x00, 0x00, 0x05, (char)0xDC,

    // 10. Tx DF (1500)
    0x00, 0x00, 0x05, (char)0xDC,

    // 11. DX Call (Length=7, "DX_CALL")
    0x00, 0x00, 0x00, 0x07, 0x44, 0x58, 0x5F, 0x43, 0x41, 0x4C, 0x4C,

    // 12. DX Grid (Length=4, "GRID")
    0x00, 0x00, 0x00, 0x04, 0x47, 0x52, 0x49, 0x44,

    // 13. Gen Msgs (True), 14. Tx Even (False)
    0x01, 0x00,

	//padding to 89 bytes
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
*/

const char raw_packet_data[97] = {
    // HEADER (12 bytes)
    // Magic (0xADBC CBDA), Schema=3, Type=17 (0x11)
    (char)0xAD, (char)0xBC, (char)0xCB, (char)0xDA, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x16,

    // 1. Session ID (11 bytes: Length=7, "WSJT-X\0") - START: 12, END: 22
    // Using length 7 is standard for Schema 3 Client IDs.
    //0x00, 0x00, 0x00, 0x07, 0x57, 0x53, 0x4A, 0x54, 0x2D, 0x59, 0x00,
    0x00, 0x00, 0x00, 0x06, 0x57, 0x53, 0x4A, 0x54, 0x2D, 0x59,

    // 2. Dial Freq (8 bytes: 28,075,000 Hz, encoded as 0x01AC63F8) - START: 23, END: 30
    //0x00, 0x00, 0x00, 0x00, 0x01, (char)0xAC, 0x63, (char)0xF8,       //28075000
    0x00, 0x00, 0x00, 0x00, 0x01, (char)0xAC, 0x60, (char)0x10,       //28074000
	//0x00, 0x00, 0x00, 0x06, 0x89, (char)0x66, 0x80, (char)0xC0,     // 28075000000   -00 00 00 06 | 89 57 3E 80 |
    //0x00, 0x00, 0x00, 0x06, 0x89, (char)0x57, 0x3E, (char)0x80,     // 28074000000

    // 3. **Tx Freq (8 bytes: ADDED, Same as Dial Freq)** - START: 31, END: 38
    //0x00, 0x00, 0x00, 0x00, 0x01, (char)0xAC, 0x63, (char)0xF8,
    //0x00, 0x00, 0x00, 0x06, 0x89, (char)0x66, 0x80, (char)0xC0,
    0x00, 0x00, 0x00, 0x06, 0x89, (char)0x57, 0x3E, (char)0x80,

    // 4. Tx Enabled (1 byte: False) - START: 39
    0x00,

    // 5. Mode (7 bytes: Length=3, "FT8") - START: 40
    0x00, 0x00, 0x00, 0x03, 0x46, 0x54, 0x38,

    // 6. Freq Tolerance (4 bytes: 2500) - START: 47
    0x00, 0x00, 0x09, (char)0xC4,

    // 7. Sub Mode (4 bytes: Length=0, "") - START: 51
    0x00, 0x00, 0x00, 0x00,

    // 8. Fast Mode (1 byte: False) - START: 55
    0x00,

    // 9. Tr Period (4 bytes: 15) - START: 56
    0x00, 0x00, 0x00, 0x0F,

    // 10. Rx DF (4 bytes: 1500) - START: 60
    0x00, 0x00, 0x05, (char)0xDC,

    // 11. Tx DF (4 bytes: 1500) - START: 64
    0x00, 0x00, 0x05, (char)0xDC,

    // 12. DX Call (11 bytes: Length=7, "DX_CALL") - START: 68
    0x00, 0x00, 0x00, 0x07, 0x44, 0x58, 0x5F, 0x43, 0x41, 0x4C, 0x4C,

    // 13. DX Grid (8 bytes: Length=4, "GRID") - START: 79
    0x00, 0x00, 0x00, 0x04, 0x47, 0x52, 0x49, 0x44,

    // 14. Gen Msgs (1 byte: True) - START: 87
    0x01,

    // 15. Tx Even (1 byte: False) - START: 88
    0x00,

    // PADDING (9 bytes: To make it 97 bytes total, if needed to match a fixed buffer size) - START: 89, END: 96
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const size_t PACKET_SIZE = sizeof(raw_packet_data); // 89


// Helper function for Big Endian serialization
// Your system is likely little-endian (x86/x64), so all multi-byte values
// must be byte-swapped before being added to the network buffer.

// Big Endian 32-bit integer (quint32)
void appendBeUint32(std::vector<char>& buffer, uint32_t value) {
    uint32_t be_value = htonl(value); // Convert to network byte order (Big Endian)
    const char* bytes = reinterpret_cast<const char*>(&be_value);
    buffer.insert(buffer.end(), bytes, bytes + sizeof(uint32_t));
}

// Big Endian 64-bit integer (quint64)
void appendBeUint64(std::vector<char>& buffer, uint64_t value) {
    // Use the MSVC intrinsic function for reliable 64-bit byte swapping
    uint64_t be_value = _byteswap_uint64(value);

    // Append the 8 bytes of the swapped value
    const char* bytes = reinterpret_cast<const char*>(&be_value);
    buffer.insert(buffer.end(), bytes, bytes + sizeof(uint64_t));
}

// QUTF8 String: [BE_Length (4 bytes)] + [UTF8 Bytes]
void appendQUtf8(std::vector<char>& buffer, const std::string& str) {
    uint32_t length = static_cast<uint32_t>(str.length());
    appendBeUint32(buffer, length);

    // CHANGE THIS LINE: Explicitly use the data pointer and length
    //buffer.insert(buffer.end(), str.data(), str.data() + length);
    buffer.insert(buffer.end(), str.data(), str.data() + length);
}

void printHexFrequency(uint64_t frequency) {

    std::vector<char> messageBuffer;
    appendBeUint64(messageBuffer, frequency);
	std::cout << "Frequency: " << frequency << " Hz, Hex: ";
    hexDumpBuffer(messageBuffer);
}

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
) {
    std::vector<char> messageBuffer;

    // --- 1. Header (Big Endian) ---
    appendBeUint32(messageBuffer, 0xADBCCBDA); // Magic
    appendBeUint32(messageBuffer, 3);          // Schema
    appendBeUint32(messageBuffer, PacketType::SetFreq); // Type: 17
    appendQUtf8(messageBuffer, sessionId);     // Client ID

    // --- 2. Type 17 Payload (Big Endian Qt Format) ---
    appendBeUint64(messageBuffer, dialFrequencyHz);
    messageBuffer.push_back(txEnabled ? 1 : 0);      // qbool
    appendQUtf8(messageBuffer, mode);                // qutf8
    appendBeUint32(messageBuffer, freqTolerance);    // quint32
    appendQUtf8(messageBuffer, subMode);
    messageBuffer.push_back(fastMode ? 1 : 0);
    appendBeUint32(messageBuffer, trPeriod);
    appendBeUint32(messageBuffer, rxDF);
    appendBeUint32(messageBuffer, txDF);
    appendQUtf8(messageBuffer, dxCall);
    appendQUtf8(messageBuffer, dxGrid);
    messageBuffer.push_back(generateMessages ? 1 : 0);
    messageBuffer.push_back(txEven ? 1 : 0);

	//hex dump for debugging
    hexDumpBuffer(messageBuffer);


    // --- 3. Send using the existing socket and destination address ---

    sockaddr_in destinationAddr;
    destinationAddr.sin_family = AF_INET;
    destinationAddr.sin_port = htons(PORT); // WSJT-X is listening on PORT (2237)

    // WSJT-X is running on the SERVER IP (127.0.0.1)
    if (inet_pton(AF_INET, SERVER, &destinationAddr.sin_addr) <= 0) {
        std::cerr << "Error: Invalid destination address\n";
        return false;
    }
/*
    int result = sendto(
        tx_socket,
        messageBuffer.data(),
        messageBuffer.size(),
        0,
        (sockaddr*)&destinationAddr,
        sizeof(destinationAddr)
    );
*/
    //hexDumpBuffer(raw_packet_data);
    int result = sendto(
        tx_socket,
        raw_packet_data,
        PACKET_SIZE,
        0,
        (sockaddr*)&destinationAddr,
        sizeof(destinationAddr)
    );

    if (result == SOCKET_ERROR) {
        std::cerr << "sendto failed: " << WSAGetLastError() << "\n";
    }
    else {
        std::cout << "Sent Set Freq (" << result << " bytes) to " << SERVER << ":" << PORT << std::endl;
    }

    return result != SOCKET_ERROR;
}

void hexDumpBuffer(const std::vector<char>& buffer) {
    std::cout << "\n--- HEX DUMP (Total Size: " << std::dec << buffer.size() << " bytes) ---\n";
    std::cout << std::hex << std::uppercase << std::setfill('0');

    for (size_t i = 0; i < buffer.size(); ++i) {
        // Use static_cast<int> and & 0xFF to treat the char as an unsigned byte
        // and avoid sign extension issues.
        std::cout << std::setw(2) << (static_cast<int>(buffer[i]) & 0xFF) << " ";

        // Add separators for readability
        if ((i + 1) % 4 == 0) std::cout << "| ";
        if ((i + 1) % 16 == 0) std::cout << "\n";
    }

    // Newline and reset stream formatting
    std::cout << "\n" << std::dec << std::setfill(' ') << std::endl;
}





/*
// Example Usage: Set FT8 on 20m, Tx Even, 1500Hz offset
int main() {
    sendSetFreqCommand(
        "VS2022_Controller",    // sessionId (Client ID)
        14074000000ULL,         // dialFrequencyHz (14.074000 MHz)
        true,                   // txEnabled
        "FT8",                  // mode
        2500,                   // freqTolerance (2500 Hz waterfall span)
        "",                     // subMode (empty)
        false,                  // fastMode
        15,                     // trPeriod (15 seconds)
        1500,                   // rxDF (RX Audio Freq)
        1500,                   // txDF (TX Audio Freq)
        "",                     // dxCall (empty)
        "",                     // dxGrid (empty)
        true,                   // generateMessages (Auto 73, RR73, etc.)
        true                    // txEven (True to TX on Even minutes)
    );
    return 0;
}
*/