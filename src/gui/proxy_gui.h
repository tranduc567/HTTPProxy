#ifndef PROXY_GUI_H
#define PROXY_GUI_H

#include <winsock2.h>
#include <windows.h>
#include <string>
#include <vector>
#include <map>
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
#define ID_COMBO_TIME_MODE 110  // ComboBox for selecting time filter mode (Time Filter)
#define ID_BTN_SET_TIMES   111  // Button to set time ranges
#define ID_BANNED_TIMES_BOX   112  // Button to set time ranges
#define IDD_TIME_DIALOG 200 



// Custom user message to log messages
#define WM_LOG_MESSAGE     (WM_USER + 1)
INT_PTR CALLBACK TimeDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// Callback function to handle window messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Function to create the main window of the application
HWND CreateMainWindow(HINSTANCE hInstance, int nCmdShow);

#endif // PROXY_GUI_H
