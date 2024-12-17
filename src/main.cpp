#include <winsock2.h>
#include <windows.h>
#include "gui/proxy_gui.h"
#include "proxy_server/proxy_server.h"
#include "content_filter/content_filter.h"
#include <thread>
#include <chrono>

BlacklistFilter blacklistFilter;
WhitelistFilter whitelistFilter;
KeywordFilter keywordFilter;
int selectedMode;
int selectedTime;
HINSTANCE hInstance = GetModuleHandle(NULL);
std::map<std::string, std::vector<std::pair<int, int>>> bannedTimes = {
    {"facebook.com", {{8, 24}}},
    {"twitter.com", {{8, 17}}},
    {"instagram.com", {{8, 17}}},
    {"tiktok.com", {{8, 17}}},
    {"youtube.com", {{7, 18}}},
    {"netflix.com", {{0, 6}, {8, 18}}},
    {"twitch.tv", {{0, 7}, {8, 18}}},
    {"hulu.com", {{0, 6}, {8, 18}}},
    {"cnn.com", {{10, 24}}},
    {"bbc.com", {{10, 24}}},
    {"reuters.com", {{10, 24}}},
    {"amazon.com", {{9, 18}}},
    {"ebay.com", {{9, 18}}},
    {"alibaba.com", {{9, 18}}},
    {"coursera.org", {}},
    {"khanacademy.org", {}},
    {"edx.org", {}},
    {"linkedin.com", {}},
    {"stackoverflow.com", {}},
    {"github.com", {}},
    {"google.com", {}},
    {"bing.com", {}},
    {"yahoo.com", {}},
    {"reddit.com", {{8, 17}}},
    {"pinterest.com", {{8, 17}}}
};

int main(int argc, char* argv[]) {
    int nCmdShow = SW_SHOWDEFAULT;
    HWND hwnd = CreateMainWindow(hInstance, nCmdShow);

    if (hwnd == NULL) {
        return 0;
    }
    ProxyServer proxyServer(8080, "upstream_host", 80, hwnd);
    std::thread serverThread([&proxyServer]() {
        proxyServer.start();
    });
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    serverThread.join();

    return 0;
}
