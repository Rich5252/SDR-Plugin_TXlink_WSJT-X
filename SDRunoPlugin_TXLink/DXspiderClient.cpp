#include <iostream>
#include <string>
#include <asio.hpp>
#include <ctime>
#include <iomanip>
#include <optional> // For std::optional
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>

#include "DXspiderClient.h"
#include "MessageBox.h"
#include "MessageQueue.h"



using asio::ip::tcp;
using asio::error_code;

std::string host = "";
unsigned short port;

asio::io_context io_context;
asio::ip::tcp::socket DXsocket(io_context);
asio::error_code error;
tcp::resolver resolver(io_context);
tcp::resolver::results_type endpoints;
bool bConnected = false;

std::string read_telnet_response(tcp::socket& DXsocket, std::string strUntil, error_code& error);

std::atomic<bool> DXspiderExitRequest = false;		//break out of loop and tidy up on app close
ThreadSafeMessageQueue<DXspot_t> DXspiderSpotOutQ;

//local functions forward defs
int DXspiderSpot(std::string dxSpiderPort, std::string my_call, std::string spot_message);   // spot message format: "DX <frequency> <dx_call> <notes>"
std::size_t flush_socket_input(tcp::socket& socket, error_code& ec);
void DXspiderDisconnect();
void ConnectDXspider(std::string DXspiderPort, std::string my_call);
std::string read_telnet_response(tcp::socket& DXsocket, std::string strUntil, error_code& error);
bool is_socket_active(asio::ip::tcp::socket& sock);
int DXspiderSpot(std::string dxSpiderPort, std::string my_call, std::string spot_message);
void DXspider_force_close();
std::size_t flush_socket_input(tcp::socket& socket, error_code& ec);


// loop runs in its own thread started by TXLink
// waits for spots to appear in the queue and sends them to DXspider
// Spot is constructed in Form when user clicks the DXspot button
// exits when DXspiderExitRequest is set true
//TODO: add force disconnect function to be called on app exit
void DXspiderLoop() {

    while (!DXspiderExitRequest) {
        if (!DXspiderSpotOutQ.empty()) {
            DXspot_t spot = DXspiderSpotOutQ.waitAndPop();
            int result = DXspiderSpot(spot.dxSpiderPort, spot.myCall, spot.DXspot);
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            asio::error_code error;
            flush_socket_input(DXsocket, error);            //flush anything sent from DXspider
		}
    }

    DXspiderDisconnect();
	DXspiderExitRequest = false;
	return;
}


void ConnectDXspider(std::string DXspiderPort ,std::string my_call) {

    //std::string host_port_str = "gb7djk.dxcluster.net:7300";
    //std::string host_port_str = "dxfun.com:8000";
    std::string host_port_str = DXspiderPort;           // "dxspider.co.uk:7300";
    

    size_t colon_pos = host_port_str.find(':');
    if (colon_pos == std::string::npos) {
        std::cerr << "DXspider: Invalid dxcluster_host:port format using " << DXspiderPort << "\n";
        return ;
    }

    host = host_port_str.substr(0, colon_pos);
    std::string port_str = host_port_str.substr(colon_pos + 1);
    
    try {
        port = std::stoul(port_str);
        if (port > 65535) {
            throw std::out_of_range("");
        }
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "DXspider: Invalid port number: " << port_str << "\n";
        return ;
    }
    catch (const std::out_of_range& e) {
        std::cerr << "DXspider: Port number out of range: " << port_str << "\n";
        return ;
    }

    try {

        endpoints = resolver.resolve(host, std::to_string(port));

		std::cout << get_millis() << "DXspider: Connecting to DXspider server at " << host << ":" << port << "\n";
        asio::error_code connect_error;
        asio::connect(DXsocket, endpoints, connect_error);

        if (connect_error) {
            std::cerr << get_millis() << "DXspider: Connection failed: " << connect_error.message() << "\n";
            return; // Exit on connection failure
        }
        
		if (DXspiderExitRequest) { return; }
        
        asio::error_code error;

        //wait for login prompt
        std::string response = read_telnet_response(DXsocket, "login:", error);
        if (error) {
            std::cerr << get_millis() << "DXspider: Error while waiting for login: prompt: " << error.message() << "\n";
            return; // Exit if error on login prompt
        }

        std::cout << get_millis() << "DXspider: login prompt response: " << response << std::endl;

        // login
		std::cout << get_millis() << "DXspider: Sending login information: " << my_call << "\n";
        std::string login_message = my_call + "\r\n";           //must include Telnet eol

        asio::write(DXsocket, asio::buffer(login_message), error);
        if (error) {
            std::cerr << get_millis() << "DXspider: Error sending login information: " << error.message() << "\n";
            return ; // Exit if login fails
        }
        else {
            std::cout << get_millis() << "DXspider: Login attempt sent.\n";

            // Read the response until we get dxspider prompt
            if (DXspiderExitRequest) { return; }

            std::string response = read_telnet_response(DXsocket, "dxspider >\r\n", error);
            std::cout << get_millis() << "DXspider: Response after login: " << response;
            if (error) {
                std::cerr << get_millis() << "DXspider: Error waiting for prompt: " << error.message() << "\n";
                return; // Exit if login fails
            }
        }
	}
	catch (std::exception& e) {
		std::cerr << get_millis() << "DXspider: Exception: " << e.what() << "\n";
	}

	bConnected = true;
	return;
}

void DXspiderDisconnect() {
	if (bConnected) {
        //disconnect
        std::string logout_command = "quit";
        logout_command = logout_command + "\r\n";
        asio::write(DXsocket, asio::buffer(logout_command), error);

        DXsocket.close();
        std::cout << get_millis() << "DXspider: Disconnected from DXspider server.\n";

		bConnected = false;
	}
	else {
		std::cout << get_millis() << "DXspider: Already disconnected from DXspider server.\n";
	}
}


std::string read_telnet_response(tcp::socket& DXsocket, std::string strUntil ,error_code& error) {
    asio::streambuf response_buf;
    asio::read_until(DXsocket, response_buf, strUntil, error);

    if (DXspiderExitRequest) { return ""; }

    if (error == asio::error::eof) {
        std::string response(asio::buffers_begin(response_buf.data()),
            asio::buffers_end(response_buf.data()));
        return response;
    }
    else if (error) {
        std::cerr << get_millis() << "DXspider: Error reading response: " << error.message() << "\n";
        return "";
    }
    else {
        std::string response(asio::buffers_begin(response_buf.data()),
            asio::buffers_end(response_buf.data()));
        return response;
    }
}

bool is_socket_active(asio::ip::tcp::socket& sock) {
    // 1. Check if the socket is open
    if (!sock.is_open()) {
        return false;
    }

    asio::error_code ec;
    // 2. Perform a zero-byte read operation
    // This tells the OS to check the connection state without waiting for data.
    // The message_peek flag means data is checked but NOT removed from the buffer.
    sock.receive(asio::null_buffers(), asio::socket_base::message_peek, ec);

    // 3. Analyze the error code
    if (ec == asio::error::eof || ec == asio::error::connection_reset) {
        // EOF means the remote side has performed an orderly shutdown.
        // connection_reset means the connection was lost abruptly.
        return false;
    }
    else if (ec) {
        // Any other fatal error (e.g., bad file descriptor, network down)
        // Log this error if appropriate.
        return false;
    }

    // No error, or error is 'would_block' (which means the connection is fine, 
    // but there's no data waiting).
    return true;
}



int DXspiderSpot(std::string dxSpiderPort, std::string my_call, std::string spot_message) {
    if (!is_socket_active(DXsocket)) { bConnected = false; }
    if (!bConnected) { ConnectDXspider(dxSpiderPort, my_call); }
    
	if (!bConnected) {
		std::cerr << get_millis() << "DXspider: Not connected to DXspider.\n";
		return 1;
	}
    
    try {
        // Construct the DX spot message
        std::string message_to_send = spot_message + "\r\n";

        // Send the DX spot
		std::cout << get_millis() << "DXspider: Sending DX spot: " << message_to_send;
        asio::write(DXsocket, asio::buffer(message_to_send), error);

        if (!error) {
            std::cout << get_millis() << "DXspider: DX spot sent successfully to " << host << ":" << port << "\n";
            std::cout << message_to_send;

            if (DXspiderExitRequest) { return 0; }

            // Read the response until we get next dxspider prompt
            asio::error_code error;
            std::string response = read_telnet_response(DXsocket, "dxspider >\r\n", error);
            std::cout << get_millis() << "DXspider: Response to spot: " << ": " << response;

            if (error) {
                std::cerr << get_millis() << "DXspider: Response error after sending DX spot: " << error.message() << "\n";
            }
        }
        else {
            std::cerr << get_millis() << "DXspider: Write error sending DX spot: " << error.message() << "\n";
        }
    }
    catch (std::exception& e) {
        std::cerr << get_millis() << "DXspider: Exception: " << e.what() << "\n";
    }

    return 0;
}


// Forcefully close the socket to cancel any pending operations
// called on app exit to ensure clean shutdown - see TXLink closure code
void DXspider_force_close() {
    // This is the key line to cancel the pending async_read_until
    asio::error_code ec;
    DXsocket.close(ec);
    // Note: The pending read completion handler will now run with asio::error::operation_aborted
}

/**
 * @brief Reads and discards all data currently available in the socket's input buffer.
 * * @param socket The TCP socket to flush.
 * @param ec The error_code object to store any error.
 * @return std::size_t The total number of bytes flushed.
 */
std::size_t flush_socket_input(tcp::socket& socket, error_code& ec) {
    ec.clear(); // Clear any previous error
    std::size_t bytes_flushed = 0;

    // Use a small, temporary buffer for reading
    std::vector<char> temp_buffer(1024);

    // 1. Set the socket to non-blocking mode temporarily
    // This ensures that the receive call returns immediately if no data is available.
    socket.non_blocking(true, ec);
    if (ec) return 0; // Failure to set non-blocking mode

    // Loop until the socket returns an "EAGAIN" or "EWOULDBLOCK" error 
    // (meaning there's no more data) or a fatal error occurs.
    while (true) {
        // 2. Perform a non-blocking synchronous receive
        std::size_t bytes_read = socket.receive(asio::buffer(temp_buffer), 0, ec);

        if (ec == asio::error::would_block || ec == asio::error::try_again) {
            // This is the success condition for the flush: no more data is available.
            ec.clear();
            break;
        }
        else if (ec) {
            // A fatal error occurred (e.g., connection reset, EOF)
            if (bConnected) {
                std::cout << "DXspider: Flush input indicates connection to DXspider has closed" << std::endl;
                bConnected = false;
            }
            break;
        }
        else if (bytes_read == 0) {
            // Usually indicates a clean EOF or a successful read of 0 bytes 
            // after the last available data, depending on the OS.
            break;
        }

        // 3. Update the total count of flushed bytes
        bytes_flushed += bytes_read;
    }

    // 4. Restore the socket to its original (default) blocking mode
    socket.non_blocking(false, ec);
    // Note: You should check ec again here if you care about the success of restoring mode.

    return bytes_flushed;
}

