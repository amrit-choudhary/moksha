#ifdef ME_WIN

/**
 * Main entry point for the Windows application.
 * This file is specific to Windows and uses the Win32 API for application management.
 */

#include <d3d12.h>
#include <dxgi1_6.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <wrl.h>

#include <atomic>
#include <cstdio>
#include <iostream>
#include <thread>

#include "moksha/client/main/main_win.h"

void AttachDebugConsole() {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    std::cout.clear();
    std::cerr.clear();
}

using Microsoft::WRL::ComPtr;

LRESULT CALLBACK WindowProcW(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_NCCREATE: {
            CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
            ME::ClientApp* pApp = reinterpret_cast<ME::ClientApp*>(pCreate->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pApp));
            break;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }

        // Handle input messages and forward them to the game instance, which gives them to the input system.
        case WM_SYSKEYUP:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_KEYDOWN:
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP: {
            ME::ClientApp* pApp = reinterpret_cast<ME::ClientApp*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
            if (pApp) {
                pApp->HandleInput(msg, wParam, lParam);
            }
            return 0;
        }
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
#ifdef _DEBUG
    AttachDebugConsole();
#endif

    // Initialize the game
    char exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);
    ME::Utils::SetPaths(exePath, nullptr);

    ME::ClientApp app;

    // Initialize the window class.
    WNDCLASSEXW windowClass = {0};
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProcW;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"Moksha";
    RegisterClassExW(&windowClass);

    // Desired client area size
    uint16_t clientWidth = 1200;
    uint16_t clientHeight = 900;
    ME::GlobalVars::SetWindowSize(clientWidth, clientHeight);

    RECT rect = {0, 0, clientWidth, clientHeight};
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);

    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;

    HWND hWnd = CreateWindowExW(0, windowClass.lpszClassName, L"Moksha", WS_OVERLAPPEDWINDOW, 60, 60, windowWidth,
                                windowHeight, nullptr, nullptr, hInstance, &app);

    app.Init(hWnd);
    ShowWindow(hWnd, nCmdShow);

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Update the game
        app.Update();
    }

    // Cleanup
    app.ShutDownGameSystems();

    FreeConsole();
    return 0;
}

#endif  // ME_WIN
