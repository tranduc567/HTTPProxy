#include "proxy_server.h"
#include <iostream>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "../content_filter/content_filter.h"
#include "../gui/proxy_gui.h"
#include <sstream>
#include <iomanip>
extern int selectedMode;
std::string getCurrentTime() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    std::ostringstream oss;

    // Use std::setw and std::setfill to ensure two digits for time components
    oss << (localTime->tm_year + 1900) << "-"
        << std::setw(2) << std::setfill('0') << (localTime->tm_mon + 1) << "-"
        << std::setw(2) << std::setfill('0') << localTime->tm_mday << " "
        << std::setw(2) << std::setfill('0') << localTime->tm_hour << ":"
        << std::setw(2) << std::setfill('0') << localTime->tm_min << ":"
        << std::setw(2) << std::setfill('0') << localTime->tm_sec;
    
    return oss.str();
}


void ProxyServer::handleClient(SOCKET clientSocket) {
    char buffer[4096]; // Buffer to hold the incoming data

    // Attempt to read data from the client
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        std::cout << "Error reading data from client or connection closed." << std::endl;
        closesocket(clientSocket);
        return;
    }

    // Convert the received data into a string
    std::string request(buffer, bytesRead);
    std::string host;

    // Extract the host from the request (looking for the "Host:" header)
    size_t hostPos = request.find("Host:");
    if (hostPos != std::string::npos) {
        size_t start = hostPos + 5; // Move past "Host:"
        size_t end = request.find("\r\n", start);
        if (end != std::string::npos) {
            host = request.substr(start, end - start);

            // Trim spaces from the host
            host.erase(0, host.find_first_not_of(" \t"));
            host.erase(host.find_last_not_of(" \t") + 1);
        }
    }

    if (host.empty()) {
        std::cout << "Host not found in request." << std::endl;
        closesocket(clientSocket);
        return;
    }

    // Get the current time for the log message
    std::string currentTime = getCurrentTime();

    if(!selectedMode){
        if (blacklistFilter.applyFilter(host)) {
            std::string logMessage = "[" + currentTime + "] Host blocked by blacklist: " + host + "\n";
            logToGUI(logMessage);
            sendRedirectResponse(clientSocket); // Send a redirect response for blocked hosts
            closesocket(clientSocket);
            return;
        }
    }
    else{
         if (!whitelistFilter.applyFilter(host)) {
            std::string logMessage = "[" + currentTime + "] Host blocked by whitelist filter: " + host + "\n";
            logToGUI(logMessage);
            sendBlockedResponse(clientSocket); // Send blocked response for non-whitelisted hosts
            closesocket(clientSocket);
            return;
        }
    }
    // // Apply Keyword Filter: Block the request if restricted keywords are found in the body
    // if (keywordFilter.applyFilter(request)) {
    //     std::string logMessage = "[" + currentTime + "] Request contains restricted keyword, blocking host: " + host + "\n";
    //     logToGUI(logMessage);
    //     sendBlockedResponse(clientSocket); // Send a blocked response for keyword matches
    //     closesocket(clientSocket);
    //     return;
    // }

    // Add host to the running hosts list
    addHostToRunningList(host + "\n");

    // Proceed with further processing of the request (HTTP or HTTPS)
    if (request.find("CONNECT") != std::string::npos) {
        std::string logMessage = "[" + currentTime + "] Processing HTTPS request from: " + host + "\n";
        logToGUI(logMessage);
        handleHTTPS(clientSocket, buffer, bytesRead);
    } else {
        std::string logMessage = "[" + currentTime + "] Processing HTTP request from: " + host + "\n";
        logToGUI(logMessage);
        handleHTTP(clientSocket, buffer, bytesRead);
    }

    closesocket(clientSocket);
}


// Helper function to send a redirect response
void ProxyServer::sendRedirectResponse(SOCKET clientSocket) {
    std::string response = "HTTP/1.1 301 Moved Permanently\r\n"
                           "Location: https://www.youtube.com/watch?v=3UMwfEsL6SM\r\n"
                           "Connection: close\r\n"
                           "Cache-Control: no-cache\r\n"
                           "\r\n";
    send(clientSocket, response.c_str(), response.size(), 0);
    std::cout << "Redirect response sent." << std::endl;
}

// Helper function to send a blocked response
void ProxyServer::sendBlockedResponse(SOCKET clientSocket) {
    std::string response = "HTTP/1.1 403 Forbidden\r\n"
                           "Connection: close\r\n"
                           "Cache-Control: no-cache\r\n"
                           "\r\n";
    send(clientSocket, response.c_str(), response.size(), 0);
    std::cout << "Blocked response sent." << std::endl;
}

void ProxyServer::handleHTTP(SOCKET clientSocket, char* buffer, int bytesRead) {
    std::string httpRequest(buffer, bytesRead);
    std::string host;

    // Parse Host header from HTTP request
    size_t hostPos = httpRequest.find("Host: ");
    if (hostPos != std::string::npos) {
        size_t hostEnd = httpRequest.find("\r\n", hostPos);
        host = httpRequest.substr(hostPos + 6, hostEnd - hostPos - 6);
    } else {
        std::cerr << "No Host header found in HTTP request" << std::endl;
        return;
    }

    size_t portPos = host.find(":");
    if (portPos != std::string::npos) {
        host = host.substr(0, portPos);
    }

    // Resolve hostname using getaddrinfo
    struct addrinfo hints = {}, *res;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(host.c_str(), "80", &hints, &res);
    if (status != 0) {
        std::cerr << "Failed to resolve hostname: " << host << std::endl;
        return;
    }

    SOCKET upstreamSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (upstreamSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create upstream socket" << std::endl;
        freeaddrinfo(res);
        return;
    }

    // Connect to the upstream server
    if (connect(upstreamSocket, res->ai_addr, res->ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to upstream server" << std::endl;
        freeaddrinfo(res);
        closesocket(upstreamSocket);
        return;
    }

    freeaddrinfo(res);
    send(upstreamSocket, buffer, bytesRead, 0);

    // Relay data between client and upstream server
    while ((bytesRead = recv(upstreamSocket, buffer, sizeof(buffer), 0)) > 0) {
        send(clientSocket, buffer, bytesRead, 0);
    }

    closesocket(upstreamSocket);
}

void ProxyServer::handleHTTPS(SOCKET clientSocket, char* buffer, int bytesRead) {
    std::string httpsRequest(buffer, bytesRead);
    std::string host;

    // Parse Host header from HTTPS request
    size_t hostPos = httpsRequest.find("Host: ");
    if (hostPos != std::string::npos) {
        size_t hostEnd = httpsRequest.find("\r\n", hostPos);
        host = httpsRequest.substr(hostPos + 6, hostEnd - hostPos - 6);
    } else {
        std::cerr << "No Host header found in HTTPS request" << std::endl;
        return;
    }

    size_t portPos = host.find(":");
    if (portPos != std::string::npos) {
        host = host.substr(0, portPos);
    }

    // Resolve hostname using getaddrinfo for HTTPS (port 443)
    struct addrinfo hints = {}, *res;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(host.c_str(), "443", &hints, &res);
    if (status != 0) {
        std::cerr << "Failed to resolve hostname: " << host << std::endl;
        return;
    }

    SOCKET upstreamSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (upstreamSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create upstream socket" << std::endl;
        freeaddrinfo(res);
        return;
    }

    // Connect to the upstream HTTPS server
    if (connect(upstreamSocket, res->ai_addr, res->ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to upstream server" << std::endl;
        freeaddrinfo(res);
        closesocket(upstreamSocket);
        return;
    }

    freeaddrinfo(res);

    // Send a response to client to establish tunnel
    const char* response = "HTTP/1.1 200 Connection Established\r\n\r\n";
    send(clientSocket, response, strlen(response), 0);

    // Relay data between the client and upstream server
    fd_set readfds;
    while (true) {
        FD_ZERO(&readfds);
        FD_SET(clientSocket, &readfds);
        FD_SET(upstreamSocket, &readfds);

        int maxFd = std::max(clientSocket, upstreamSocket) + 1;
        if (select(maxFd, &readfds, nullptr, nullptr, nullptr) < 0) {
            std::cerr << "select() error" << std::endl;
            break;
        }

        // Forward data from client to upstream server
        if (FD_ISSET(clientSocket, &readfds)) {
            bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead <= 0) break;
            send(upstreamSocket, buffer, bytesRead, 0);
        }

        // Forward data from upstream server to client
        if (FD_ISSET(upstreamSocket, &readfds)) {
            bytesRead = recv(upstreamSocket, buffer, sizeof(buffer), 0);
            if (bytesRead <= 0) break;
            send(clientSocket, buffer, bytesRead, 0);
        }
    }
    // Clean up
    closesocket(upstreamSocket);
    closesocket(clientSocket);
}
