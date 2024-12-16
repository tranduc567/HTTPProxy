#ifndef PROXY_GUI_H
#define PROXY_GUI_H

#include <winsock2.h>
#include <windows.h>
#include <string>
#include <vector>

// ID for the interface elements
#define ID_LIST_BLACKLIST  101
#define ID_LIST_HOSTS      102
#define ID_LOGBOX          103
#define ID_BTN_ADD         104
#define ID_BTN_DELETE      105
#define ID_BTN_STOP        106
#define ID_BTN_CLEAR       107
#define ID_BTN_HELP        108
#define ID_COMBO_MODE      109  // ComboBox for selecting mode (Blacklist or Whitelist)

// Custom user message to log messages
#define WM_LOG_MESSAGE     (WM_USER + 1)

// Callback function to handle window messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Function to create the main window of the application
HWND CreateMainWindow(HINSTANCE hInstance, int nCmdShow);

#endif // PROXY_GUI_H
