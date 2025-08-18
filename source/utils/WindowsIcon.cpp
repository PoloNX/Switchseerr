#include "utils/WindowsIcon.hpp"

#ifdef _WIN32
#include "utils/resource.h"
#include <string>
#include <thread>
#include <chrono>

HWND FindApplicationWindow() {
    // Try multiple approaches to find the application window
    HWND hwnd = nullptr;
    
    // First try by window title
    hwnd = FindWindowA(nullptr, "Switchseerr");
    if (hwnd != nullptr) return hwnd;
    
    // Try alternative titles that Borealis might use
    hwnd = FindWindowA(nullptr, "Borealis");
    if (hwnd != nullptr) return hwnd;
    
    // Try by process - get current process windows
    DWORD processId = GetCurrentProcessId();
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        DWORD windowProcessId;
        GetWindowThreadProcessId(hwnd, &windowProcessId);
        if (windowProcessId == (DWORD)lParam) {
            // Check if this is a main window (has title bar, visible, etc.)
            if (IsWindowVisible(hwnd) && (GetWindowLong(hwnd, GWL_STYLE) & WS_CAPTION)) {
                *((HWND*)&lParam) = hwnd; // Store found window
                return FALSE; // Stop enumeration
            }
        }
        return TRUE; // Continue enumeration
    }, (LPARAM)processId);
    
    return hwnd;
}

void SetApplicationIcon() {
    HWND hwnd = nullptr;
    
    // Retry finding the window for up to 5 seconds
    for (int i = 0; i < 50; ++i) {
        hwnd = FindApplicationWindow();
        if (hwnd != nullptr) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    if (hwnd == nullptr) {
        return;
    }

    // Load the icon from resources
    HICON hIcon = (HICON)LoadImage(
        GetModuleHandle(nullptr),   // Instance handle
        MAKEINTRESOURCE(IDI_ICON),  // Resource identifier
        IMAGE_ICON,                 // Image type
        0, 0,                      // Width and height (0 = default size)
        LR_DEFAULTSIZE | LR_SHARED // Flags
    );

    if (hIcon != nullptr) {
        // Set both large and small icons
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        
        // Also set the class icon for future windows
        SetClassLongPtr(hwnd, GCLP_HICON, (LONG_PTR)hIcon);
        SetClassLongPtr(hwnd, GCLP_HICONSM, (LONG_PTR)hIcon);
        
        // Force a redraw of the window frame
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, 
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
}

#endif // _WIN32
