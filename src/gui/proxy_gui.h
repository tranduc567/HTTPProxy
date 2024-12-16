#ifndef PROXY_GUI_H
#define PROXY_GUI_H
#include <winsock2.h>
#include <windows.h>
#include <unordered_set>
#include <string>
#include <vector>

// Định nghĩa ID cho các thành phần giao diện
#define ID_LIST_BLACKLIST  101
#define ID_LIST_HOSTS      102
#define ID_LOGBOX          103
#define ID_BTN_ADD         104
#define ID_BTN_DELETE      105
#define ID_BTN_STOP        106
#define ID_BTN_CLEAR       107
#define ID_BTN_HELP        108  // ID cho nút Help

// Hàm xử lý cửa sổ chính
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Hàm tạo cửa sổ chính
HWND CreateMainWindow(HINSTANCE hInstance, int nCmdShow);

#endif // PROXY_GUI_H
