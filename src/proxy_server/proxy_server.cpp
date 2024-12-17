#include "proxy_server.h"
#include <iostream>
#include <memory> 
#include <stdexcept>
#include <thread>
#include <cstring>
#include "../content_filter/content_filter.h" 
#include "../gui/proxy_gui.h" 

extern std::map<std::string, std::vector<std::pair<int, int>>> bannedTimes;

ProxyServer::ProxyServer(int port, const std::string& upstreamHost, int upstreamPort, HWND hWndGUI)
    : port(port), upstreamHost(upstreamHost), upstreamPort(upstreamPort),hWndGUI(hWndGUI),
      serverSocket(INVALID_SOCKET),runningHosts(){};       

void ProxyServer::start() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("Failed to initialize Winsock");
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR ||
        listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        throw std::runtime_error("Failed to bind or listen on socket");
    }

    logToGUI("Proxy server listening on port " + port);

    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }

        std::thread(&ProxyServer::handleClient, this, clientSocket).detach();
    }
}


void ProxyServer::cleanup() {
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
    }
    WSACleanup();
}

ProxyServer::~ProxyServer() {
    cleanup();
}
void ProxyServer::logToGUI(const std::string& message) {
    if (hWndGUI) {
        HWND hLogBox = GetDlgItem(hWndGUI, ID_LOGBOX);
        if (hLogBox) {
            SendMessage(hLogBox, EM_REPLACESEL, FALSE, (LPARAM)message.c_str());
            SendMessage(hLogBox, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
        }
    }
}

void ProxyServer::addHostToRunningList(const std::string& host) {
    if (hWndGUI) {
        HWND hHosts = GetDlgItem(hWndGUI, ID_LIST_HOSTS);
        if (hHosts) {
            SendMessage(hHosts, EM_REPLACESEL, FALSE, (LPARAM)host.c_str());
            SendMessage(hHosts, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
        }
    }
}
int ProxyServer::getCurrentHour() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    return localTime->tm_hour;
}

bool ProxyServer::isAccessBanned(const std::string& url) {
    int currentHour = getCurrentHour();
    for (const auto& entry : bannedTimes) {
        const std::string& bannedUrl = entry.first;
        if (url.find(bannedUrl) != std::string::npos) {
            for (const auto& timeRange : entry.second) {
                int startHour = timeRange.first;
                int endHour = timeRange.second;
                if ((startHour < endHour && currentHour >= startHour && currentHour < endHour) ||
                    (startHour >= endHour && (currentHour >= startHour || currentHour < endHour))) {
                    return true;  // Nếu có, trả về true để chặn URL
                }
            }
        }
    }
    return false;  
}
