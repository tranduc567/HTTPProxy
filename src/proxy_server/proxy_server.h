#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <string>
#include <winsock2.h>
#include <windows.h>
#include <memory>
#include "../content_filter/content_filter.h"
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <ctime>


// ---------------- Global Filters ----------------
extern BlacklistFilter blacklistFilter;
extern WhitelistFilter whitelistFilter;
extern std::map<std::string, std::vector<std::pair<int, int>>> bannedTimes;

class ProxyServer {
public:
    ProxyServer(int port, const std::string& upstreamHost, int upstreamPort,HWND hWndGUI); // Thêm tham số hWndGUI
    ~ProxyServer();
    void start();
    void handleClient(SOCKET clientSocket);
    void cleanup();
    void sendRedirectResponse(SOCKET clientSocket);
    void sendBlockedResponse(SOCKET clientSocket);
    void handleHTTP(SOCKET clientSocket, char* buffer, int bytesRead);
    void handleHTTPS(SOCKET clientSocket, char* buffer, int bytesRead);
    bool isAccessBanned(const std::string& url);
    int getCurrentHour();
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
    HWND hWndGUI;  
};


#endif

