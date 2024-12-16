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

int main(int argc, char* argv[]) {
    HINSTANCE hInstance = GetModuleHandle(NULL);
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
