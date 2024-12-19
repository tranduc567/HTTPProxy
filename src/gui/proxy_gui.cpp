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
        hAddBtn = CreateWindowA("button", "Save List", WS_CHILD | WS_VISIBLE,
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
        int controlWidth = (windowWidth - 100) / 2; // Khoảng cách giữa hai control và lề là 50
        int controlHeight = windowHeight / 3 - 40;
        int logBoxWidth = windowWidth - 250; // Để lại khoảng cách cho ô BannedTimes
        int logBoxHeight = windowHeight / 3 - 40;
        int buttonWidth = 180;
        int buttonHeight = 45;
        int buttonSpacing = 20; // Khoảng cách giữa các nút

        // Di chuyển các control
        MoveWindow(hBlacklist, 30, 60, controlWidth, controlHeight, TRUE);
        MoveWindow(hHosts, windowWidth - controlWidth - 30, 60, controlWidth, controlHeight, TRUE);
        MoveWindow(hLogBox, 30, windowHeight / 2 - 10, logBoxWidth, logBoxHeight, TRUE);
        MoveWindow(hBannedTimesBox, windowWidth - 200, windowHeight / 2 - 10, 150, logBoxHeight - 10, TRUE); // Di chuyển ô BannedTimesBox

        // Di chuyển các nút
        int buttonY = windowHeight - 80;
        MoveWindow(hAddBtn, 30, buttonY, buttonWidth, buttonHeight, TRUE);
        MoveWindow(hDeleteBtn, 30 + buttonWidth + buttonSpacing, buttonY, buttonWidth, buttonHeight, TRUE);
        MoveWindow(hStopBtn, 30 + 2 * (buttonWidth + buttonSpacing), buttonY, buttonWidth, buttonHeight, TRUE);
        MoveWindow(hHelpBtn, 30 + 3 * (buttonWidth + buttonSpacing), buttonY, buttonWidth, buttonHeight, TRUE);

        // Di chuyển tiêu đề
        MoveWindow(hHostsTitle, windowWidth - controlWidth - 30, 20, controlWidth, 30, TRUE);
        MoveWindow(hLogBoxTitle, 30, windowHeight / 2 - 50, logBoxWidth, 30, TRUE);
        MoveWindow(hBannedTimesTitle, windowWidth - 200, windowHeight / 2 - 50, 150, 30, TRUE); // Title cho ô BannedTimes

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
               MessageBoxW(
                NULL,
                L"Proxy Server GUI - Hướng dẫn sử dụng:\n\n"
                L"1. Chọn chế độ (Blacklist hoặc Whitelist) từ menu thả xuống phía trên cùng bên trái.\n"
                L"2. Thêm các địa chỉ vào danh sách tương ứng bằng cách nhập vào ô bên dưới và nhấn 'Save List'.\n"
                L"3. Chọn chế độ Time Filter (On/Off) để kích hoạt hoặc tắt bộ lọc thời gian.\n"
                L"4. Sử dụng các nút sau:\n"
                L"   - Save List: Lưu danh sách vào Blacklist hoặc Whitelist.\n"
                L"   - Delete All: Xóa toàn bộ nội dung trong các ô danh sách.\n"
                L"   - Stop Proxy: Dừng proxy server và đóng ứng dụng.\n"
                L"   - Help: Hiển thị hướng dẫn sử dụng này.\n\n"
                L"5. Các khu vực hiển thị:\n"
                L"   - Hosts Running: Danh sách các host đang chạy.\n"
                L"   - Log Box: Hiển thị các thông báo nhật ký hoạt động.\n"
                L"   - Banned Times List: Danh sách thời gian bị cấm (giúp bạn lành mạnh hơn).\n\n"
                L"Lưu ý: Thay đổi được áp dụng tự động sau khi bạn lưu danh sách.",
                L"Help",
                MB_OK | MB_HELP
            );
        break;

        default:
            break;
        }
        break;
    }
    case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, hBrushBackground);
            return 1; 
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
