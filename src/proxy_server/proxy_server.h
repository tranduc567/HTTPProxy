#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <string>
#include <winsock2.h>
#include <windows.h>
#include <memory>
#include "../content_filter/content_filter.h"
#include <unordered_set>


// ---------------- Global Filters ----------------
extern BlacklistFilter blacklistFilter;
extern WhitelistFilter whitelistFilter;
extern KeywordFilter keywordFilter;

class ProxyServer {
public:
    ProxyServer(int port, const std::string& upstreamHost, int upstreamPort,HWND hWndGUI); // Thêm tham số hWndGUI
    ~ProxyServer();
    void start();
    void handleClient(SOCKET clientSocket);
    void cleanup();
    void sendRedirectResponse(SOCKET clientSocket);
    void handleBlacklistManagement(SOCKET clientSocket, const std::string& request);
    void sendBlockedResponse(SOCKET clientSocket);
    void handleHTTP(SOCKET clientSocket, char* buffer, int bytesRead);
    void handleHTTPS(SOCKET clientSocket, char* buffer, int bytesRead);

    const std::unordered_set<std::string>& getRunningHosts() const {
        return runningHosts;
    }
    void addHostToRunningList(const std::string& host);
    void logToGUI(const std::string& message); // Thêm hàm logToGUI
private:
    int port;
    std::string upstreamHost;
    int upstreamPort;
    SOCKET serverSocket;
    std::unordered_set<std::string> runningHosts;
    HWND hWndGUI;  // Thêm hWndGUI để kết nối GUI
};

#endif
 // PROXY_SERVER_H
