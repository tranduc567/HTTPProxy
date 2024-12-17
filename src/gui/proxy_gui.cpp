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
extern HINSTANCE hInstance;
extern std::map<std::string, std::vector<std::pair<int, int>>> bannedTimes;
extern int selectedMode;
extern int selectedTime;

// Hàm xử lý giao diện
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hBlacklist, hHosts, hLogBox, hBannedTimesBox, hAddBtn, hDeleteBtn, hStopBtn, hHelpBtn, hModeComboBox, hTimeModeComboBox, hSetTimeBtn;
    static HWND hHostsTitle, hLogBoxTitle, hBannedTimesTitle;
    static HBRUSH hBrushBackground;
    static int windowWidth, windowHeight;
    switch (uMsg) {
        case WM_CREATE: {
        hModeComboBox = CreateWindowA("combobox", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | WS_VSCROLL,
            30, 30, 200, 100, hwnd, (HMENU)ID_COMBO_MODE, NULL, NULL);
        SendMessageA(hModeComboBox, CB_ADDSTRING, 0, (LPARAM)"Blacklist");
        SendMessageA(hModeComboBox, CB_ADDSTRING, 0, (LPARAM)"Whitelist");
        SendMessageA(hModeComboBox, CB_SETCURSEL, 0, 0); // Mặc định chọn Blacklist
        hTimeModeComboBox = CreateWindowA("combobox", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | WS_VSCROLL,
            240, 30, 200, 100, hwnd, (HMENU)ID_COMBO_TIME_MODE, NULL, NULL);
        SendMessageA(hTimeModeComboBox, CB_ADDSTRING, 0, (LPARAM)"Time Filter Off");
        SendMessageA(hTimeModeComboBox, CB_ADDSTRING, 0, (LPARAM)"Time Filter On");
        SendMessageA(hTimeModeComboBox, CB_SETCURSEL, 0, 1); // Mặc định là Off
        hBlacklist = CreateWindowA("edit", NULL, 
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL, 
        30, 60, 400, 200, hwnd, (HMENU)ID_LIST_BLACKLIST, NULL, NULL);
        
        hHosts = CreateWindowA("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOHSCROLL | ES_READONLY,
            450, 60, 400, 200, hwnd, (HMENU)ID_LIST_HOSTS, NULL, NULL);
        
        hLogBox = CreateWindowA("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
            30, 280, 550, 200, hwnd, (HMENU)ID_LOGBOX, NULL, NULL); // Giảm chiều cao

        hBannedTimesBox = CreateWindowA("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
            600, 280, 250, 200, hwnd, (HMENU)ID_BANNED_TIMES_BOX, NULL, NULL); // Thêm ô mới cho BannedTimes

        HFONT hFont = CreateFont(
            15, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH,
            "Times New Roman"
        );
        SendMessage(hLogBox, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(hBannedTimesBox, WM_SETFONT, (WPARAM)hFont, TRUE);
        
        hLogBoxTitle = CreateWindowA("static", "Log Box", WS_CHILD | WS_VISIBLE,
            30, 250, 550, 30, hwnd, NULL, NULL, NULL); // LogBoxTitle cho ô LogBox
        hBannedTimesTitle = CreateWindowA("static", "Banned Times List(Helping you healthier)", WS_CHILD | WS_VISIBLE,
            600, 250, 250, 30, hwnd, NULL, NULL, NULL);  // Title cho ô mới

        hHostsTitle = CreateWindowA("static", "Host Running", WS_CHILD | WS_VISIBLE,
            450, 30, 400, 30, hwnd, NULL, NULL, NULL);  // Đặt HostsTitle ở vị trí thích hợp
        hAddBtn = CreateWindowA("button", "Update List", WS_CHILD | WS_VISIBLE,
            30, 500, 190, 45, hwnd, (HMENU)ID_BTN_ADD, NULL, NULL);
        hDeleteBtn = CreateWindowA("button", "Delete All", WS_CHILD | WS_VISIBLE,
            240, 500, 220, 45, hwnd, (HMENU)ID_BTN_DELETE, NULL, NULL);
        hStopBtn = CreateWindowA("button", "Stop Proxy", WS_CHILD | WS_VISIBLE,
            480, 500, 180, 45, hwnd, (HMENU)ID_BTN_STOP, NULL, NULL);
        hHelpBtn = CreateWindowA("button", "Help", WS_CHILD | WS_VISIBLE,
            670, 500, 180, 45, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);

        std::string bannedTimesList = "[Banned Times List]\r\n";
        for (const auto& entry : bannedTimes) {
            bannedTimesList += entry.first + ":\r\n";
            for (const auto& time : entry.second) {
                bannedTimesList += "  " + std::to_string(time.first) + ":00 to " + std::to_string(time.second) + ":00\r\n";
            }
        }
        SendMessageA(hBannedTimesBox, WM_SETTEXT, 0, (LPARAM)bannedTimesList.c_str());
        SetTimer(hwnd, 1, 2000, NULL);  // Timer ID là 1, khoảng thời gian 1000 ms
        break;
    }

    case WM_SIZE: {
        // Cập nhật kích thước cửa sổ
        windowWidth = LOWORD(lParam);
        windowHeight = HIWORD(lParam);

        // Tính toán lại kích thước và vị trí các control
        int controlWidth = windowWidth / 2 - 40;
        int controlHeight = windowHeight / 3 - 40;
        int logBoxHeight = windowHeight / 3 - 40;

        // Di chuyển các control
        MoveWindow(hBlacklist, 30, 60, controlWidth, controlHeight, TRUE);
        MoveWindow(hHosts, windowWidth / 2 + 20, 60, controlWidth, controlHeight, TRUE);
        MoveWindow(hLogBox, 30, windowHeight / 2 - 10, 550, logBoxHeight, TRUE);
        MoveWindow(hBannedTimesBox, 600, windowHeight / 2 - 10, 250, logBoxHeight - 10, TRUE); // Di chuyển ô BannedTimesBox

        MoveWindow(hAddBtn, 30, windowHeight - 80, 180, 45, TRUE);
        MoveWindow(hDeleteBtn, 230, windowHeight - 80, 180, 45, TRUE);
        MoveWindow(hStopBtn, 430, windowHeight - 80, 180, 45, TRUE);
        MoveWindow(hHelpBtn, 620, windowHeight - 80, 180, 45, TRUE);

        MoveWindow(hHostsTitle, windowWidth / 2 + 20, 20, controlWidth, 30, TRUE);
        MoveWindow(hLogBoxTitle, 30, windowHeight / 2 - 50, 550, 30, TRUE);
        MoveWindow(hBannedTimesTitle, 600, windowHeight / 2 - 50, 250, 30, TRUE);  // Title cho ô BannedTimes

        break;
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
         case ID_COMBO_TIME_MODE: { 
            selectedTime = SendMessageA(hTimeModeComboBox, CB_GETCURSEL, 0, 0); 
            const char* mode = (selectedTime == 1) ? "[INFO] Time Filter On selected.\r\n" : "[INFO] Time Filter Off selected.\r\n";
            SendMessageA(hLogBox, EM_REPLACESEL, 0, (LPARAM)mode);
            break;
        }
        case ID_COMBO_MODE: {
            // Đọc giá trị của ComboBox để xác định chế độ
            selectedMode = SendMessageA(hModeComboBox, CB_GETCURSEL, 0, 0);
            const char* mode = (selectedMode == 0) ? "[INFO] Blacklist mode selected.\r\n" : "[INFO] Whitelist mode selected.\r\n";
            SendMessageA(hLogBox, EM_REPLACESEL, 0, (LPARAM)mode);
            break;
        }

        case ID_BTN_ADD: {
            char buffer[256] = {0};
            HWND hEditInput = GetDlgItem(hwnd, ID_LIST_BLACKLIST);
            if (hEditInput) {
                int lineCount = SendMessageA(hBlacklist, EM_GETLINECOUNT, 0, 0);

                // Determine mode based on ComboBox selection
                selectedMode = SendMessageA(hModeComboBox, CB_GETCURSEL, 0, 0);
                if (selectedMode == 0) {
                    blacklistFilter.clear();
                    for (int i = 0; i < lineCount; i++) {
                        *((WORD*)buffer) = sizeof(buffer) - 1;
                        int lineLength = SendMessageA(hEditInput, EM_GETLINE, i, (LPARAM)buffer);
                        if (lineLength > 0) {
                            buffer[lineLength] = '\0';
                            blacklistFilter.addToBlacklist(buffer);
                            std::string logMessage = "[LOG] Added to Blacklist: " + std::string(buffer) + "\r\n";
                            SendMessageA(hLogBox, EM_REPLACESEL, 0, (LPARAM)logMessage.c_str());
                        }
                    }
                    SendMessageA(hLogBox, EM_REPLACESEL, 0, (LPARAM)"[INFO] Blacklist updated.\r\n");

                } else {
                    whitelistFilter.clear();
                    for (int i = 0; i < lineCount; i++) {
                        *((WORD*)buffer) = sizeof(buffer) - 1;
                        int lineLength = SendMessageA(hEditInput, EM_GETLINE, i, (LPARAM)buffer);

                        if (lineLength > 0) {
                            buffer[lineLength] = '\0'; 
                            whitelistFilter.addToWhitelist(buffer);
                            std::string logMessage = "[LOG] Added to Whitelist: " + std::string(buffer) + "\r\n";
                            SendMessageA(hLogBox, EM_REPLACESEL, 0, (LPARAM)logMessage.c_str());
                        }
                    }
                    SendMessageA(hLogBox, EM_REPLACESEL, 0, (LPARAM)"[INFO] Whitelist updated.\r\n");
                }
            } else {
                SendMessageA(hLogBox, EM_REPLACESEL, 0, (LPARAM)"[ERROR] Cannot access input control.\r\n");
            }
            break;
        }

        case ID_BTN_DELETE:
            SendMessageA(hBlacklist, WM_SETTEXT, 0, (LPARAM)"");
            SendMessageA(hHosts, WM_SETTEXT, 0, (LPARAM)"");
            SendMessageA(hLogBox, WM_SETTEXT, 0, (LPARAM)"");
            SendMessageA(hLogBox, EM_REPLACESEL, 0, (LPARAM)"[LOG] All lists have been cleared.\r\n");
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
                "1. Add to List: Adds a host to the blacklist or whitelist based on the selected mode.\n"
                "2. Delete from List: Removes a host from the list.\n"
                "3. Stop Proxy: Stops the proxy server.\n",
                "Help", MB_OK | MB_HELP);
            break;

        default:
            break;
        }
        break;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        HWND hControl = (HWND)lParam;
        if (hControl == hHostsTitle) {
            SetTextColor(hdcStatic, RGB(255, 0, 0));  // Set text color to red
            SetBkColor(hdcStatic, RGB(240, 240, 240));  // Set background color to light gray
            return (LRESULT)hBrushBackground;
        }

        // Check if the control is the LogBox (to set text color to blue)
        if (hControl == hLogBox) {
            SetTextColor(hdcStatic, RGB(0, 0, 255));  // Set text color to blue
            SetBkColor(hdcStatic, RGB(240, 240, 240));  // Set background color to light gray
            return (LRESULT)hBrushBackground;
        }

        // For other static controls, use the default colors
        SetTextColor(hdcStatic, RGB(255, 0, 0));  // Default text color (blue)
        SetBkColor(hdcStatic, RGB(240, 240, 240));  // Default background color
        return (LRESULT)hBrushBackground;
    }

    case WM_CTLCOLORBTN: {
         HDC hdcButton = (HDC)wParam;
        HWND hButton = (HWND)lParam;
        if (hButton == hAddBtn) {
            SetTextColor(hdcButton, RGB(0, 128, 0));  // Green text
            SetBkColor(hdcButton, RGB(144, 238, 144));  // Light Green background
        }
        else if (hButton == hDeleteBtn) {
            SetTextColor(hdcButton, RGB(255, 0, 0));  // Red text
            SetBkColor(hdcButton, RGB(255, 182, 193));  // Light Pink background
        }
        else if (hButton == hStopBtn) {
            SetTextColor(hdcButton, RGB(0, 0, 255));  // Blue text
            SetBkColor(hdcButton, RGB(173, 216, 230));  // Light Blue background
        }
        else if (hButton == hHelpBtn) {
            SetTextColor(hdcButton, RGB(128, 0, 128));  // Purple text
            SetBkColor(hdcButton, RGB(230, 230, 250));  // Light Lavender background
        } else {
            SetTextColor(hdcButton, RGB(0, 0, 0));  // Default black text
            SetBkColor(hdcButton, RGB(255, 255, 255));  // White background
        }

        SetBkMode(hdcButton, TRANSPARENT);  // Make sure background is transparent
        return (LRESULT)hBrushBackground;
    }

    case WM_CTLCOLOREDIT: {
        HDC hdcEdit = (HDC)wParam;
        HWND hControl = (HWND)lParam;
        if (hControl == hBlacklist) {
            SetTextColor(hdcEdit, RGB(0, 0, 0));  // Black text color
            SetBkColor(hdcEdit, RGB(255, 255, 204));  // Light yellow background
            return (LRESULT)hBrushBackground;
        }
        break;
    }
    case WM_DESTROY:
        DeleteObject(hBrushBackground);
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
