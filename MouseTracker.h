#pragma once

#ifdef MOUSETRACKER_EXPORTS
#define MOUSETRACKER_API __declspec(dllexport)
#else
#define MOUSETRACKER_API __declspec(dllimport)
#endif

extern "C" {
    MOUSETRACKER_API void startMouseTracking();
    MOUSETRACKER_API void stopMouseTracking();
    MOUSETRACKER_API int getAccumulatedDeltaX();
    MOUSETRACKER_API int getAccumulatedDeltaY();
    MOUSETRACKER_API int getAccumulatedScroll();
    MOUSETRACKER_API bool isLeftButtonDown();
    MOUSETRACKER_API bool isRightButtonDown();
    MOUSETRACKER_API bool isMiddleButtonDown();
    MOUSETRACKER_API bool isXButton1Down();
    MOUSETRACKER_API bool isXButton2Down();
}
