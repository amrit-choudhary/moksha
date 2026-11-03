#ifdef ME_WEB

/**
 * Web entry and frame driver. A device arrives asynchronously, so main only asks for it and returns; the app starts in
 * the callback, once the device exists. The browser then calls Tick once per display refresh. The page can close at any
 * moment, so nothing depends on Shutdown running.
 */

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <webgpu/webgpu.h>

#include <algorithm>
#include <cmath>
#include <string>

#include "moksha/client/app/client_app.h"
#include "moksha/client/platform/web/web_webgpu_view.h"
#include "moksha/logging/logging.h"
#include "moksha/shared/misc/utils.h"

namespace {

constexpr const char* CANVAS = "#canvas";
constexpr int NO_TOUCH = -1;

// Everything the browser's callbacks need; handed to each of them as its user data.
class WebApp {
   public:
    ME::ClientApp app;
    ME::WebWebGPUView view;
    WGPUInstance instance = nullptr;
    // CSS pixels per window unit: the page shrinks a game that doesn't fit.
    float cssScale = 1.0f;
    int primaryTouch = NO_TOUCH;
    // Set once the game asked to quit (Tick returned false); a page can't close itself, so it only logs.
    bool gameQuit = false;
};

std::string ToString(WGPUStringView text) {
    if (text.data == nullptr) {
        return {};
    }
    return text.length == WGPU_STRLEN ? std::string{text.data} : std::string{text.data, text.length};
}

// Shows a line of text in the page, above the canvas.
void ShowFailure() {
    EM_ASM({ mokshaMessage("Couldn't start WebGPU. See the browser console."); });
}

void Frame(void* userData) {
    WebApp* web = static_cast<WebApp*>(userData);
    if (!web->app.Tick() && !web->gameQuit) {
        web->gameQuit = true;
        ME::Log("Web: the game asked to quit; a page keeps running until it is closed");
    }
}

bool OnKey(int eventType, const EmscriptenKeyboardEvent* event, void* userData) {
    static_cast<WebApp*>(userData)->app.HandleKeyEvent(event->code, eventType == EMSCRIPTEN_EVENT_KEYDOWN);
    // Keep game keys from scrolling the page, but leave browser shortcuts and function keys to the browser.
    const bool isFunctionKey = event->code[0] == 'F' && event->code[1] >= '0' && event->code[1] <= '9';
    return !event->ctrlKey && !event->metaKey && !event->altKey && !isFunctionKey;
}

bool OnMouseMove(int, const EmscriptenMouseEvent* event, void* userData) {
    WebApp* web = static_cast<WebApp*>(userData);
    web->app.HandleMouseMove(event->targetX / web->cssScale, event->targetY / web->cssScale);
    return false;
}

// DOM buttons are 0 left, 1 middle, 2 right; the engine's are 0 left, 1 right, 2 middle.
int ToEngineButton(int domButton) {
    return domButton == 1 ? 2 : (domButton == 2 ? 1 : domButton);
}

bool OnMouseDown(int, const EmscriptenMouseEvent* event, void* userData) {
    WebApp* web = static_cast<WebApp*>(userData);
    web->app.HandleMouseMove(event->targetX / web->cssScale, event->targetY / web->cssScale);
    web->app.HandleMouseButton(ToEngineButton(event->button), true);
    return false;
}

bool OnMouseUp(int, const EmscriptenMouseEvent* event, void* userData) {
    static_cast<WebApp*>(userData)->app.HandleMouseButton(ToEngineButton(event->button), false);
    return false;
}

// The first finger on the canvas is the mouse, as on iOS. Consumed, so the browser doesn't also send mouse events.
bool OnTouch(int eventType, const EmscriptenTouchEvent* event, void* userData) {
    WebApp* web = static_cast<WebApp*>(userData);
    for (int i = 0; i < event->numTouches; ++i) {
        const EmscriptenTouchPoint& touch = event->touches[i];
        if (!touch.isChanged) {
            continue;
        }
        if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART && web->primaryTouch == NO_TOUCH) {
            web->primaryTouch = touch.identifier;
        }
        if (touch.identifier != web->primaryTouch) {
            continue;
        }
        const bool isDown = eventType == EMSCRIPTEN_EVENT_TOUCHSTART || eventType == EMSCRIPTEN_EVENT_TOUCHMOVE;
        web->app.HandleMouseMove(touch.targetX / web->cssScale, touch.targetY / web->cssScale);
        web->app.HandleMouseButton(0, isDown);
        if (!isDown) {
            web->primaryTouch = NO_TOUCH;
        }
    }
    return true;
}

bool OnBlur(int, const EmscriptenFocusEvent*, void* userData) {
    static_cast<WebApp*>(userData)->app.ReleaseAllInput();
    return false;
}

bool OnVisibilityChange(int, const EmscriptenVisibilityChangeEvent* event, void* userData) {
    WebApp* web = static_cast<WebApp*>(userData);
    if (event->hidden) {
        web->app.ReleaseAllInput();
        web->app.Pause();
    } else {
        web->app.Resume();
    }
    return false;
}

void RegisterEvents(WebApp* web) {
    const char* window = EMSCRIPTEN_EVENT_TARGET_WINDOW;
    emscripten_set_keydown_callback(window, web, false, OnKey);
    emscripten_set_keyup_callback(window, web, false, OnKey);
    emscripten_set_mousemove_callback(CANVAS, web, false, OnMouseMove);
    emscripten_set_mousedown_callback(CANVAS, web, false, OnMouseDown);
    // On the window, so a button released outside the canvas still counts.
    emscripten_set_mouseup_callback(window, web, false, OnMouseUp);
    emscripten_set_touchstart_callback(CANVAS, web, false, OnTouch);
    emscripten_set_touchmove_callback(CANVAS, web, false, OnTouch);
    emscripten_set_touchend_callback(CANVAS, web, false, OnTouch);
    emscripten_set_touchcancel_callback(CANVAS, web, false, OnTouch);
    emscripten_set_blur_callback(window, web, false, OnBlur);
    emscripten_set_visibilitychange_callback(web, false, OnVisibilityChange);
}

// The device exists: size the canvas and start the app.
void StartApp(WebApp* web) {
    ME::AppArgs args;
    const ME::AppConfig& config = web->app.Configure(args);

    // The game keeps its configured size, and the page shrinks it to fit the browser window when needed.
    const float pageWidth = static_cast<float>(EM_ASM_INT({ return window.innerWidth; }));
    const float pageHeight = static_cast<float>(EM_ASM_INT({ return window.innerHeight; }));
    const float windowWidth = static_cast<float>(config.windowWidth);
    const float windowHeight = static_cast<float>(config.windowHeight);
    web->cssScale = std::min({1.0f, pageWidth / windowWidth, pageHeight / windowHeight});

    const float cssWidth = windowWidth * web->cssScale;
    const float cssHeight = windowHeight * web->cssScale;
    const float pixelRatio = static_cast<float>(emscripten_get_device_pixel_ratio());
    web->view.pixelWidth = static_cast<uint32_t>(std::lround(cssWidth * pixelRatio));
    web->view.pixelHeight = static_cast<uint32_t>(std::lround(cssHeight * pixelRatio));
    emscripten_set_element_css_size(CANVAS, cssWidth, cssHeight);
    emscripten_set_canvas_element_size(CANVAS, static_cast<int>(web->view.pixelWidth),
                                       static_cast<int>(web->view.pixelHeight));

    web->app.SetWindowMetrics(config.windowWidth, config.windowHeight,
                              static_cast<float>(web->view.pixelWidth) / windowWidth);

    if (!web->app.Init(&web->view)) {
        ME::LogError("Web: the app failed to initialize");
        web->app.Shutdown();
        ShowFailure();
        return;
    }

    RegisterEvents(web);
    EM_ASM({ mokshaMessage(""); });
    emscripten_set_main_loop_arg(Frame, web, 0, false);
}

void OnUncapturedError(WGPUDevice const*, WGPUErrorType, WGPUStringView message, void*, void*) {
    ME::LogError("WebGPU: ", ToString(message));
}

void OnDeviceLost(WGPUDevice const*, WGPUDeviceLostReason reason, WGPUStringView message, void*, void*) {
    // Destroyed means the page itself is going away, which isn't worth reporting.
    if (reason != WGPUDeviceLostReason_Destroyed && reason != WGPUDeviceLostReason_CallbackCancelled) {
        ME::LogError("WebGPU: device lost: ", ToString(message));
    }
}

void OnDevice(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void* userData, void*) {
    WebApp* web = static_cast<WebApp*>(userData);
    if (status != WGPURequestDeviceStatus_Success) {
        ME::LogError("WebGPU: no device: ", ToString(message));
        ShowFailure();
        return;
    }
    web->view.device = device;
    StartApp(web);
}

void OnAdapter(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* userData, void*) {
    WebApp* web = static_cast<WebApp*>(userData);
    if (status != WGPURequestAdapterStatus_Success) {
        ME::LogError("WebGPU: no adapter: ", ToString(message));
        ShowFailure();
        return;
    }
    web->view.adapter = adapter;

    WGPUDeviceDescriptor deviceDescriptor = WGPU_DEVICE_DESCRIPTOR_INIT;
    deviceDescriptor.uncapturedErrorCallbackInfo.callback = OnUncapturedError;
    deviceDescriptor.deviceLostCallbackInfo.mode = WGPUCallbackMode_AllowSpontaneous;
    deviceDescriptor.deviceLostCallbackInfo.callback = OnDeviceLost;

    WGPURequestDeviceCallbackInfo callback = WGPU_REQUEST_DEVICE_CALLBACK_INFO_INIT;
    callback.mode = WGPUCallbackMode_AllowSpontaneous;
    callback.callback = OnDevice;
    callback.userdata1 = web;
    wgpuAdapterRequestDevice(adapter, &deviceDescriptor, callback);
}

}  // namespace

int main() {
    if (EM_ASM_INT({ return navigator.gpu ? 1 : 0; }) == 0) {
        ME::LogError("Web: this browser has no WebGPU");
        EM_ASM({ mokshaMessage("This browser doesn't support WebGPU, which this game needs."); });
        return 0;
    }

    // The cooked resources are preloaded into the in-memory file system at /resources before main runs.
    ME::Utils::AppPaths paths;
    paths.executableDir = "/";
    paths.resources = "/resources/";
    paths.dlc = "/dlc/";
    paths.userData = "/user_data/";
    ME::Utils::SetPaths(paths);

    // Lives for the whole page: callbacks and the frame loop keep using it after main returns.
    WebApp* web = new WebApp();
    web->instance = wgpuCreateInstance(nullptr);

    WGPUEmscriptenSurfaceSourceCanvasHTMLSelector canvasSource =
        WGPU_EMSCRIPTEN_SURFACE_SOURCE_CANVAS_HTML_SELECTOR_INIT;
    canvasSource.selector = {CANVAS, WGPU_STRLEN};
    WGPUSurfaceDescriptor surfaceDescriptor = WGPU_SURFACE_DESCRIPTOR_INIT;
    surfaceDescriptor.nextInChain = &canvasSource.chain;
    web->view.surface = wgpuInstanceCreateSurface(web->instance, &surfaceDescriptor);

    WGPURequestAdapterCallbackInfo callback = WGPU_REQUEST_ADAPTER_CALLBACK_INFO_INIT;
    callback.mode = WGPUCallbackMode_AllowSpontaneous;
    callback.callback = OnAdapter;
    callback.userdata1 = web;
    wgpuInstanceRequestAdapter(web->instance, nullptr, callback);
    return 0;
}

#endif  // ME_WEB
