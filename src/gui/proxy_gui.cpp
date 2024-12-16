#include "proxy_gui.h"
#include <vector>
#include <string>
#include <iostream>
#include <windows.h>
#include <commctrl.h>  // For control styles like ES_MULTILINE, ES_READONLY, etc.
#include "../proxy_server/proxy_server.h"
#include "../content_filter/content_filter.h"

// Sử dụng extern để tham chiếu đến các biến toàn cục
extern BlacklistFilter blacklistFilter;
extern WhitelistFilter whitelistFilter;
extern KeywordFilter keywordFilter;

#define WM_LOG_MESSAGE (WM_USER + 1)  

// Hàm xử lý giao diện
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hBlacklist, hHosts, hLogBox, hAddBtn, hDeleteBtn, hStopBtn, hHelpBtn;
    static HWND hBlacklistTitle, hHostsTitle, hLogBoxTitle;
    static int windowWidth, windowHeight;

    switch (uMsg) {
    case WM_CREATE: {
        hBlacklist = CreateWindowA("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL,
            30, 60, 400, 200, hwnd, (HMENU)ID_LIST_BLACKLIST, NULL, NULL);
        hBlacklistTitle = CreateWindowA("static", "Blacklist", WS_CHILD | WS_VISIBLE,
            30, 30, 400, 30, hwnd, NULL, NULL, NULL);

        hHosts = CreateWindowA("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL | ES_READONLY,
            450, 60, 400, 200, hwnd, (HMENU)ID_LIST_HOSTS, NULL, NULL);
        hHostsTitle = CreateWindowA("static", "Host Running", WS_CHILD | WS_VISIBLE,
            450, 30, 400, 30, hwnd, NULL, NULL, NULL);

        hLogBox = CreateWindowA("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
            30, 280, 820, 200, hwnd, (HMENU)ID_LOGBOX, NULL, NULL);
                        // Set font for the LogBox (non-bold)
        HFONT hFont = CreateFont(
            15,            // Font height
            0,             // Font width (0 means automatic width)
            0,             // Angle
            0,             // Angle
            FW_NORMAL,     // Font weight (FW_NORMAL is non-bold)
            0,             // Italic (0 means false)
            0,             // Underline (0 means false)
            0,             // Strikeout (0 means false)
            ANSI_CHARSET,  // Character set
            OUT_DEFAULT_PRECIS,   // Output precision
            CLIP_DEFAULT_PRECIS,  // Clipping precision
            DEFAULT_QUALITY,      // Quality
            DEFAULT_PITCH,        // Pitch
            "Times New Roman"            // Font name
        );
        // Apply the font to the LogBox
        SendMessage(hLogBox, WM_SETFONT, (WPARAM)hFont, TRUE);
        hLogBoxTitle = CreateWindowA("static", "Log Box", WS_CHILD | WS_VISIBLE,
            30, 250, 820, 30, hwnd, NULL, NULL, NULL);

        hAddBtn = CreateWindowA("button", "Add to Blacklist", WS_CHILD | WS_VISIBLE,
            30, 500, 190, 45, hwnd, (HMENU)ID_BTN_ADD, NULL, NULL);
        hDeleteBtn = CreateWindowA("button", "Delete from Blacklist", WS_CHILD | WS_VISIBLE,
            240, 500, 220, 45, hwnd, (HMENU)ID_BTN_DELETE, NULL, NULL);
        hStopBtn = CreateWindowA("button", "Stop Proxy", WS_CHILD | WS_VISIBLE,
            480, 500, 180, 45, hwnd, (HMENU)ID_BTN_STOP, NULL, NULL);
        hHelpBtn = CreateWindowA("button", "Help", WS_CHILD | WS_VISIBLE,
            670, 500, 180, 45, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);

        SetTimer(hwnd, 1, 1000, NULL);  // Timer ID là 1, khoảng thời gian 1000 ms
        break;
    }

    case WM_SIZE: {
        // Cập nhật kích thước cửa sổ
        windowWidth = LOWORD(lParam);
        windowHeight = HIWORD(lParam);

        // Thay đổi kích thước và vị trí các control
        int controlWidth = windowWidth / 2 - 40;
        int controlHeight = windowHeight / 3 - 40;
        int logBoxHeight = windowHeight / 3 - 40;

        MoveWindow(hBlacklist, 30, 60, controlWidth, controlHeight, TRUE);
        MoveWindow(hHosts, windowWidth / 2 + 20, 60, controlWidth, controlHeight, TRUE);
        MoveWindow(hLogBox, 30, windowHeight / 2 - 10, windowWidth - 50, logBoxHeight, TRUE);

        MoveWindow(hAddBtn, 30, windowHeight - 100, 190, 45, TRUE);
        MoveWindow(hDeleteBtn, 240, windowHeight - 100, 220, 45, TRUE);
        MoveWindow(hStopBtn, 480, windowHeight - 100, 180, 45, TRUE);
        MoveWindow(hHelpBtn, 670, windowHeight - 100, 180, 45, TRUE);

        MoveWindow(hBlacklistTitle, 30, 20, controlWidth, 30, TRUE);
        MoveWindow(hHostsTitle, windowWidth / 2 + 20, 20, controlWidth, 30, TRUE);
        MoveWindow(hLogBoxTitle, 30, windowHeight / 2 - 50, windowWidth - 60, 30, TRUE);

        break;
    }

    case WM_COMMAND: {
        int wmId = LOWORD(wParam);

        switch (wmId) {
        case ID_BTN_ADD: {
            char buffer[256] = {0};
            HWND hEditInput = GetDlgItem(hwnd, ID_LIST_BLACKLIST);
            if (hEditInput) {
                int lineCount = SendMessageA(hBlacklist, EM_GETLINECOUNT, 0, 0);
                blacklistFilter.clear();
                for (int i = 0; i < lineCount; i++) {
                    *((WORD*)buffer) = sizeof(buffer) - 1;
                    int lineLength = SendMessageA(hEditInput, EM_GETLINE, i, (LPARAM)buffer);
                    if (lineLength > 0) {
                        std::cout << buffer << "\n";
                        buffer[lineLength] = '\0'; 
                        blacklistFilter.addToBlacklist(buffer);
                        std::string logMessage = "[LOG] Added to Blacklist: " + std::string(buffer) + "\r\n";
                        SendMessageA(hLogBox, EM_REPLACESEL, 0, (LPARAM)logMessage.c_str());
                    }
                }
                SendMessageA(hLogBox, EM_REPLACESEL, 0, (LPARAM)"[INFO] Blacklist updated without clearing input.\r\n");
            } else {
                SendMessageA(hLogBox, EM_REPLACESEL, 0, (LPARAM)"[ERROR] Cannot access input control.\r\n");
            }
            break;
        }


        case ID_BTN_DELETE:
            SendMessageA(hLogBox, EM_REPLACESEL, 0, (LPARAM)"[LOG] Delete from Blacklist clicked.\r\n");
            break;

        case ID_BTN_STOP: {
            int result = MessageBoxA(hwnd,
                "Proxy server will stop. Do you want to close the proxy?",
                "Stopping Proxy",
                MB_YESNO | MB_ICONWARNING);

            if (result == IDYES) {
                MessageBoxA(hwnd, "The application will now close.", "Exiting", MB_OK | MB_ICONINFORMATION);
                PostQuitMessage(0);
                ExitProcess(0);
            } else {
                MessageBoxA(hwnd, "The application will remain running.", "Aborted", MB_OK | MB_ICONINFORMATION);
            }
            break;
        }

        case ID_BTN_HELP:
            MessageBoxA(hwnd,
                "This is a Proxy Server GUI.\n"
                "1. Add to Blacklist: Adds a host to the blacklist.\n"
                "2. Delete from Blacklist: Removes a host from the blacklist.\n"
                "3. Stop Proxy: Stops the proxy server.\n",
                "Help", MB_OK | MB_HELP);
            break;

        default:
            break;
        }
        break;
    }
    case WM_DESTROY:
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
HWND CreateMainWindow(HINSTANCE hInstance, int nCmdShow) {
    const char CLASS_NAME[] = "Proxy GUI";

    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(0, CLASS_NAME, "Proxy Server GUI", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 860, 650, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        return NULL;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    return hwnd;
}
