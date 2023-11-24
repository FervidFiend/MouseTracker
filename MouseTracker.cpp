#include "MouseTracker.h"
#include <windows.h>
#include <thread>
#include <iostream>

std::thread trackingThread;
BOOL keepRunning = TRUE;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int accumulatedDeltaX = 0;
int accumulatedDeltaY = 0;
int accumulatedScroll = 0; // New variable to store accumulated scroll amount
bool leftButtonDown = false; // New variable to store left button status
bool rightButtonDown = false; // New variable to store right button status
bool middleButtonDown = false; // New variable to store middle button status
bool xButton1Down = false; // New variable to store XBUTTON1 (Mouse 4) status
bool xButton2Down = false; // New variable to store XBUTTON2 (Mouse 5) status

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INPUT: {
        UINT dwSize;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
        LPBYTE lpb = new BYTE[dwSize];
        if (lpb == NULL) {
            return 0;
        }

        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
            std::cerr << "GetRawInputData doesn't return correct size!" << std::endl;
        }

        RAWINPUT* raw = (RAWINPUT*)lpb;
        if (raw->header.dwType == RIM_TYPEMOUSE) {
            accumulatedDeltaX += raw->data.mouse.lLastX;
            accumulatedDeltaY += raw->data.mouse.lLastY;

            // Check mouse button states
            leftButtonDown = (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) ? true : (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) ? false : leftButtonDown;
            rightButtonDown = (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) ? true : (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) ? false : rightButtonDown;
            middleButtonDown = (raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) ? true : (raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) ? false : middleButtonDown;
            xButton1Down = (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) ? true : (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP) ? false : xButton1Down;
            xButton2Down = (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) ? true : (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP) ? false : xButton2Down;

            // Check scroll wheel movement
            if (raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL) {
                accumulatedScroll += (short)raw->data.mouse.usButtonData; // usButtonData is a USHORT, need to cast to short to get the sign
            }
        }

        delete[] lpb;
        return 0;
    }
    case WM_CLOSE: {
        PostQuitMessage(0);
        return 0;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

void trackingMain() {
    const wchar_t CLASS_NAME[] = L"RawInputClass";
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"InvisibleRawInput", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, GetModuleHandle(NULL), NULL);
    if (hwnd == NULL) {
        return;
    }

    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;
    rid.dwFlags = RIDEV_INPUTSINK;  // Receive input even when this window is not in the foreground
    rid.hwndTarget = hwnd;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));

    MSG msg = {};
    while (keepRunning && GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

extern "C" {
    MOUSETRACKER_API void startMouseTracking() {
        keepRunning = TRUE;
        trackingThread = std::thread(trackingMain);
    }

    MOUSETRACKER_API void stopMouseTracking() {
        keepRunning = FALSE;
        if (trackingThread.joinable()) {
            trackingThread.join();
        }
    }

    MOUSETRACKER_API int getAccumulatedDeltaX() {
        int deltaX = accumulatedDeltaX;
        accumulatedDeltaX = 0;
        return deltaX;
    }

    MOUSETRACKER_API int getAccumulatedDeltaY() {
        int deltaY = accumulatedDeltaY;
        accumulatedDeltaY = 0;
        return deltaY;
    }

    MOUSETRACKER_API int getAccumulatedScroll() {
        int scroll = (accumulatedScroll) / 120;
        accumulatedScroll = 0;
        return scroll;
    }

    MOUSETRACKER_API bool isLeftButtonDown() {
        return leftButtonDown;
    }

    MOUSETRACKER_API bool isRightButtonDown() {
        return rightButtonDown;
    }

    MOUSETRACKER_API bool isMiddleButtonDown() {
        return middleButtonDown;
    }

    MOUSETRACKER_API bool isXButton1Down() {
        return xButton1Down;
    }

    MOUSETRACKER_API bool isXButton2Down() {
        return xButton2Down;
    }
}
