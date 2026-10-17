#ifdef ME_WIN

/**
 * Windows entry and frame driver: creates the window from the app's config, pumps messages and repeats
 * ClientApp::Tick until the window closes or the game quits. While minimized it waits instead of spinning.
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "moksha/client/app/client_app.h"
#include "moksha/shared/misc/utils.h"
#include "moksha/shared/platform/exe_path.h"

namespace {

void AttachDebugConsole() {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    std::cout.clear();
    std::cerr.clear();
}

std::wstring ToWide(const std::string& text) {
    const int length = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (length <= 0) {
        return std::wstring{};
    }
    std::wstring wide(static_cast<size_t>(length), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wide.data(), length);
    wide.resize(static_cast<size_t>(length - 1));  // drop the terminating 0 written by the conversion
    return wide;
}

ME::ClientApp* GetApp(HWND hwnd) {
    return reinterpret_cast<ME::ClientApp*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
}

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

        case WM_SIZE: {
            ME::ClientApp* pApp = GetApp(hwnd);
            if (pApp != nullptr) {
                if (wParam == SIZE_MINIMIZED) {
                    pApp->Pause();
                } else {
                    pApp->Resume();
                }
            }
            break;
        }

        // Input messages go to the app, which gives them to the input system.
        case WM_SYSKEYUP:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_KEYDOWN:
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP: {
            ME::ClientApp* pApp = GetApp(hwnd);
            if (pApp != nullptr) {
                pApp->HandleInput(msg, wParam, lParam);
            }
            return 0;
        }
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// Handles one message. False once WM_QUIT arrives.
bool DispatchOne(MSG& msg) {
    if (msg.message == WM_QUIT) {
        return false;
    }
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
    return true;
}

}  // namespace

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
#ifdef _DEBUG
    AttachDebugConsole();
#endif

    // The CRT has already split the command line into __argc/__argv.
    std::string exePath = ME::Platform::GetExecutablePath(__argc > 0 ? __argv[0] : nullptr);
    ME::Utils::SetPaths(exePath.data(), nullptr);

    ME::AppArgs args;
    args.Set(__argc, __argv);

    ME::ClientApp app;
    const ME::AppConfig& config = app.Configure(args);

    WNDCLASSEXW windowClass = {0};
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProcW;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"MokshaWindow";
    RegisterClassExW(&windowClass);

    // The config's size is the client area; grow the window by its borders and title bar.
    RECT rect = {0, 0, config.windowWidth, config.windowHeight};
    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);
    const int windowWidth = rect.right - rect.left;
    const int windowHeight = rect.bottom - rect.top;

    const std::wstring title = ToWide(config.windowTitle);
    HWND hWnd = CreateWindowExW(0, windowClass.lpszClassName, title.c_str(), WS_OVERLAPPEDWINDOW, 60, 60, windowWidth,
                                windowHeight, nullptr, nullptr, hInstance, &app);
    if (hWnd == nullptr || !app.Init(hWnd)) {
        app.Shutdown();
        return 1;
    }
    ShowWindow(hWnd, nCmdShow);

    bool running = true;
    while (running) {
        MSG msg = {};

        // Minimized: nothing to draw, so sleep until the next message instead of spinning.
        if (IsIconic(hWnd)) {
            if (GetMessageW(&msg, nullptr, 0, 0) <= 0) {
                break;
            }
            DispatchOne(msg);
            continue;
        }

        while (running && PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            running = DispatchOne(msg);
        }
        if (!running) {
            break;
        }

        if (!app.Tick()) {
            // The game asked to quit; destroying the window posts WM_QUIT, which ends the loop.
            DestroyWindow(hWnd);
        }
    }

    app.Shutdown();

    FreeConsole();
    return 0;
}

#endif  // ME_WIN
