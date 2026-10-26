#ifdef ME_MAC

/**
 * macOS entry and frame driver: creates the window from the app's config, pumps events and repeats
 * ClientApp::Tick until the window closes or the game quits. While minimized it waits instead of spinning.
 */

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>

#include <string>

#include "moksha/client/app/client_app.h"
#include "moksha/logging/logging.h"
#include "moksha/shared/misc/utils.h"
#include "moksha/shared/platform/exe_path.h"

// Window and app events (close, Cmd+Q, minimize): the macOS side of win_main's WindowProcW.
@interface MEWindowDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@property(nonatomic, assign) ME::ClientApp* app;
@property(nonatomic, assign) bool quitRequested;
@end

@implementation MEWindowDelegate

- (void)windowWillClose:(NSNotification*)notification {
    self.quitRequested = true;
}

// Cmd+Q: end the loop instead of letting AppKit exit the process, so the app shuts down as on a window close.
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
    self.quitRequested = true;
    return NSTerminateCancel;
}

- (void)windowDidMiniaturize:(NSNotification*)notification {
    if (self.app != nullptr) {
        self.app->Pause();
    }
}

- (void)windowDidDeminiaturize:(NSNotification*)notification {
    if (self.app != nullptr) {
        self.app->Resume();
    }
}

@end

// The window's content view, which Metal draws into. Input events go to the app, which gives them to the input system.
@interface MEView : MTKView
@property(nonatomic, assign) ME::ClientApp* app;
// Set once the game asked to quit (Tick returned false).
@property(nonatomic, assign) bool gameQuit;
@end

@implementation MEView

// One frame. MTKView calls this from draw, the only place it hands out the frame's drawable; overriding it (rather
// than setting a delegate) is how a subclass draws.
- (void)drawRect:(NSRect)dirtyRect {
    if (self.app != nullptr && !self.app->Tick()) {
        self.gameQuit = true;
    }
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

// The click that focuses the window also reaches the game.
- (BOOL)acceptsFirstMouse:(NSEvent*)event {
    return YES;
}

// AppKit views default to bottom-left/Y-up; flipping makes this view top-left/Y-down, the
// engine's mouse-position convention on every platform.
- (BOOL)isFlipped {
    return YES;
}

// Overriding keyDown without calling super also stops the system beep for unhandled keys.
- (void)keyDown:(NSEvent*)event {
    if (self.app != nullptr) {
        self.app->HandleKeyEvent(event.keyCode, true);
    }
}

- (void)keyUp:(NSEvent*)event {
    if (self.app != nullptr) {
        self.app->HandleKeyEvent(event.keyCode, false);
    }
}

// Modifier keys arrive as flag changes, not key downs. The flags don't tell left from right, so releasing one side
// while the other is held reads as still down.
- (void)flagsChanged:(NSEvent*)event {
    if (self.app == nullptr) {
        return;
    }
    NSEventModifierFlags flag = 0;
    switch (event.keyCode) {
        case 0x38:  // kVK_Shift
        case 0x3C:  // kVK_RightShift
            flag = NSEventModifierFlagShift;
            break;
        case 0x3B:  // kVK_Control
        case 0x3E:  // kVK_RightControl
            flag = NSEventModifierFlagControl;
            break;
        case 0x3A:  // kVK_Option
        case 0x3D:  // kVK_RightOption
            flag = NSEventModifierFlagOption;
            break;
        case 0x37:  // kVK_Command
        case 0x36:  // kVK_RightCommand
            flag = NSEventModifierFlagCommand;
            break;
        case 0x39:  // kVK_CapsLock
            flag = NSEventModifierFlagCapsLock;
            break;
        default:
            return;
    }
    self.app->HandleKeyEvent(event.keyCode, (event.modifierFlags & flag) != 0);
}

// Mouse buttons: NSEvent numbers them 0 left, 1 right, 2 middle, 3-4 extra.
- (void)mouseDown:(NSEvent*)event {
    [self forwardButton:event isDown:true];
}
- (void)mouseUp:(NSEvent*)event {
    [self forwardButton:event isDown:false];
}
- (void)rightMouseDown:(NSEvent*)event {
    [self forwardButton:event isDown:true];
}
- (void)rightMouseUp:(NSEvent*)event {
    [self forwardButton:event isDown:false];
}
- (void)otherMouseDown:(NSEvent*)event {
    [self forwardButton:event isDown:true];
}
- (void)otherMouseUp:(NSEvent*)event {
    [self forwardButton:event isDown:false];
}

- (void)forwardButton:(NSEvent*)event isDown:(bool)isDown {
    if (self.app != nullptr) {
        [self forwardPosition:event];
        self.app->HandleMouseButton(static_cast<int>(event.buttonNumber), isDown);
    }
}

// Mouse position, in view points.
- (void)mouseMoved:(NSEvent*)event {
    [self forwardPosition:event];
}
- (void)mouseDragged:(NSEvent*)event {
    [self forwardPosition:event];
}
- (void)rightMouseDragged:(NSEvent*)event {
    [self forwardPosition:event];
}
- (void)otherMouseDragged:(NSEvent*)event {
    [self forwardPosition:event];
}

- (void)forwardPosition:(NSEvent*)event {
    if (self.app != nullptr) {
        const NSPoint point = [self convertPoint:event.locationInWindow fromView:nil];
        self.app->HandleMouseMove(static_cast<float>(point.x), static_cast<float>(point.y));
    }
}

@end

namespace {

// The Metal renderer's access to the view (rendering/metal/metal_view.h). The Objective-C objects are handed over as
// their metal-cpp types, which wrap the same objects; ownership stays with the view.
class MTKMetalView : public ME::MetalView {
   public:
    explicit MTKMetalView(MTKView* inView) : view(inView) {}

    MTL::Device* GetDevice() const override {
        return reinterpret_cast<MTL::Device*>((__bridge void*)view.device);
    }

    CA::MetalLayer* GetLayer() const override {
        return reinterpret_cast<CA::MetalLayer*>((__bridge void*)view.layer);
    }

    MTL4::RenderPassDescriptor* GetCurrentRenderPass() const override {
        return reinterpret_cast<MTL4::RenderPassDescriptor*>((__bridge void*)view.currentMTL4RenderPassDescriptor);
    }

    CA::MetalDrawable* GetCurrentDrawable() const override {
        return reinterpret_cast<CA::MetalDrawable*>((__bridge void*)view.currentDrawable);
    }

   private:
    MTKView* view = nil;
};

NSString* ToNSString(const std::string& text) {
    NSString* string = [NSString stringWithUTF8String:text.c_str()];
    return string != nil ? string : @"";
}

// The app menu, with Quit (Cmd+Q).
void CreateMenuBar() {
    NSMenu* mainMenu = [[NSMenu alloc] init];
    NSMenuItem* appMenuItem = [[NSMenuItem alloc] init];
    [mainMenu addItem:appMenuItem];

    NSMenu* appMenu = [[NSMenu alloc] initWithTitle:@""];
    NSString* quitTitle = [@"Quit " stringByAppendingString:[[NSProcessInfo processInfo] processName]];
    [appMenu addItemWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"];
    [appMenuItem setSubmenu:appMenu];

    [NSApp setMainMenu:mainMenu];
}

// The next queued event, or nil. With wait, sleeps until one arrives.
NSEvent* NextEvent(bool wait) {
    return [NSApp nextEventMatchingMask:NSEventMaskAny
                              untilDate:wait ? [NSDate distantFuture] : [NSDate distantPast]
                                 inMode:NSDefaultRunLoopMode
                                dequeue:YES];
}

}  // namespace

int main(int argc, char* argv[]) {
    @autoreleasepool {
        std::string exePath = ME::Platform::GetExecutablePath(argc > 0 ? argv[0] : nullptr);
        ME::Utils::SetPaths(exePath.data(), nullptr);

        ME::AppArgs args;
        args.Set(argc, argv);

        ME::ClientApp app;
        const ME::AppConfig& config = app.Configure(args);

        // A regular app (Dock icon, menu bar, key focus) even when started as a plain executable.
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        MEWindowDelegate* windowDelegate = [[MEWindowDelegate alloc] init];
        [NSApp setDelegate:windowDelegate];
        CreateMenuBar();
        [NSApp finishLaunching];

        // The config's size is the content area, in points.
        const NSRect contentRect = NSMakeRect(0, 0, config.windowWidth, config.windowHeight);
        const NSWindowStyleMask style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                                        NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
        NSWindow* window = [[NSWindow alloc] initWithContentRect:contentRect
                                                       styleMask:style
                                                         backing:NSBackingStoreBuffered
                                                           defer:NO];
        [window setReleasedWhenClosed:NO];
        [window setTitle:ToNSString(config.windowTitle)];
        [window setAcceptsMouseMovedEvents:YES];
        [window setDelegate:windowDelegate];
        [window center];

        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if (device == nil) {
            ME::LogError("macOS: no Metal device");
            return 1;
        }

        // Drawn only when the loop calls draw, never by the view's own timer. sRGB like the DX12 back buffer.
        MEView* view = [[MEView alloc] initWithFrame:contentRect device:device];
        [view setColorPixelFormat:MTLPixelFormatBGRA8Unorm_sRGB];
        [view setPaused:YES];
        [view setEnableSetNeedsDisplay:NO];
        [window setContentView:view];
        [window makeFirstResponder:view];

        MTKMetalView metalView(view);
        if (!app.Init(&metalView)) {
            app.Shutdown();
            return 1;
        }
        windowDelegate.app = &app;
        view.app = &app;
        [window makeKeyAndOrderFront:nil];
        if (@available(macOS 14.0, *)) {
            [NSApp activate];
        } else {
            [NSApp activateIgnoringOtherApps:YES];
        }

        while (!windowDelegate.quitRequested) {
            @autoreleasepool {
                // Minimized: nothing to draw, so sleep until the next event instead of spinning.
                if ([window isMiniaturized]) {
                    NSEvent* event = NextEvent(true);
                    if (event != nil) {
                        [NSApp sendEvent:event];
                    }
                    continue;
                }

                NSEvent* event = nil;
                while (!windowDelegate.quitRequested && (event = NextEvent(false)) != nil) {
                    [NSApp sendEvent:event];
                }
                if (windowDelegate.quitRequested) {
                    break;
                }

                // Runs app.Tick through the view's drawRect. Waiting for the drawable paces the loop, as Present does
                // on Windows.
                [view draw];
                if (view.gameQuit) {
                    // The game asked to quit; closing the window sets quitRequested, which ends the loop.
                    [window close];
                }
            }
        }

        windowDelegate.app = nullptr;
        view.app = nullptr;
        app.Shutdown();
    }
    return 0;
}

#endif  // ME_MAC
