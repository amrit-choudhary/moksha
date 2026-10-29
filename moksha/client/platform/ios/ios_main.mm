#ifdef ME_IOS

/**
 * iOS entry and frame driver. UIApplicationMain runs the app with one window scene (the iOS 27 SDK requires the
 * scene lifecycle; the scene delegate is named in the engine's Info.plist template). The view controller owns the
 * ClientApp; its MTKView runs on its own display timer and calls Tick from drawRect:. iOS ends apps without notice,
 * so nothing depends on Shutdown running.
 */

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <UIKit/UIKit.h>

#include <crt_externs.h>

#include <string>

#include "moksha/client/app/client_app.h"
#include "moksha/client/platform/ios/ios_metal_view.h"
#include "moksha/logging/logging.h"
#include "moksha/shared/misc/utils.h"

namespace {

// Resources are the cook's output in the bundle's data/ folder. Writable roots live in Application Support, which is
// backed up and hidden from the user; DLC can be downloaded again, so it is kept out of backups.
ME::Utils::AppPaths MakeIosPaths() {
    ME::Utils::AppPaths paths;
    NSBundle* bundle = [NSBundle mainBundle];
    paths.executableDir = std::string{bundle.bundlePath.UTF8String};
    paths.resources = std::string{bundle.resourcePath.UTF8String} + "/data/";

    NSFileManager* files = [NSFileManager defaultManager];
    NSURL* support = [files URLForDirectory:NSApplicationSupportDirectory
                                   inDomain:NSUserDomainMask
                          appropriateForURL:nil
                                     create:YES
                                      error:nil];
    if (support == nil) {
        ME::LogError("iOS: no Application Support folder; saves and DLC are unavailable");
        return paths;
    }
    NSURL* userData = [support URLByAppendingPathComponent:@"user_data" isDirectory:YES];
    NSURL* dlc = [support URLByAppendingPathComponent:@"dlc" isDirectory:YES];
    [files createDirectoryAtURL:userData withIntermediateDirectories:YES attributes:nil error:nil];
    [files createDirectoryAtURL:dlc withIntermediateDirectories:YES attributes:nil error:nil];
    [dlc setResourceValue:@YES forKey:NSURLIsExcludedFromBackupKey error:nil];

    paths.userData = std::string{userData.path.UTF8String} + "/";
    paths.dlc = std::string{dlc.path.UTF8String} + "/";
    return paths;
}

}  // namespace

// The game's view, which Metal draws into. The first finger on it goes to the app as the mouse.
@interface MEView : MTKView
@property(nonatomic, assign) ME::ClientApp* app;
@property(nonatomic, weak) UITouch* primaryTouch;
// Set once the game asked to quit (Tick returned false); iOS apps don't end themselves, so it only logs.
@property(nonatomic, assign) bool gameQuit;
@end

@implementation MEView

// One frame. MTKView calls this on its display timer, the only place it hands out the frame's drawable.
- (void)drawRect:(CGRect)rect {
    if (self.app != nullptr && !self.app->Tick() && !self.gameQuit) {
        self.gameQuit = true;
        ME::Log("iOS: the game asked to quit; iOS apps keep running until the user closes them");
    }
}

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    if (self.primaryTouch == nil) {
        self.primaryTouch = touches.anyObject;
        [self forwardPrimaryTouch:true];
    }
}

- (void)touchesMoved:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    if (self.primaryTouch != nil && [touches containsObject:self.primaryTouch]) {
        [self forwardPrimaryTouch:true];
    }
}

- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    if (self.primaryTouch != nil && [touches containsObject:self.primaryTouch]) {
        [self forwardPrimaryTouch:false];
        self.primaryTouch = nil;
    }
}

// The system took the touch (e.g. a gesture or an alert); reads as a release.
- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    [self touchesEnded:touches withEvent:event];
}

// Position in view points, top-left origin: the engine's mouse convention.
- (void)forwardPrimaryTouch:(bool)isDown {
    if (self.app != nullptr) {
        const CGPoint point = [self.primaryTouch locationInView:self];
        self.app->HandlePrimaryTouch(static_cast<float>(point.x), static_cast<float>(point.y), isDown);
    }
}

@end

// Owns the app and its view. The app starts at the first layout, once the view has its size and screen scale.
@interface MEViewController : UIViewController
// Pauses or resumes the game and the view's timer, as the scene leaves or regains the foreground.
- (void)setAppActive:(bool)active;
// Ends the game; the scene is going away.
- (void)shutdownApp;
@end

@implementation MEViewController {
    ME::ClientApp* app;
    ME::IosMetalView* metalView;
    bool started;
}

- (void)loadView {
    // Paused until the app is initialized; then it draws on its own timer, 60 times a second. sRGB like the DX12
    // back buffer.
    MEView* view = [[MEView alloc] initWithFrame:CGRectZero device:MTLCreateSystemDefaultDevice()];
    view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    view.preferredFramesPerSecond = 60;
    view.enableSetNeedsDisplay = NO;
    view.paused = YES;
    self.view = view;
}

- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    if (!started) {
        [self startApp];
    }
}

- (void)startApp {
    MEView* view = (MEView*)self.view;
    const CGSize size = view.bounds.size;
    if (size.width <= 0.0 || size.height <= 0.0) {
        return;
    }
    started = true;
    if (view.device == nil) {
        ME::LogError("iOS: no Metal device");
        return;
    }

    ME::Utils::SetPaths(MakeIosPaths());

    // argv lives for the whole process, as AppArgs needs; UIApplicationMain doesn't pass it on.
    ME::AppArgs args;
    args.Set(*_NSGetArgc(), *_NSGetArgv());

    app = new ME::ClientApp();
    app->Configure(args);
    app->SetWindowMetrics(static_cast<uint16_t>(size.width), static_cast<uint16_t>(size.height),
                          static_cast<float>(view.contentScaleFactor));

    metalView = new ME::IosMetalView(view);
    if (!app->Init(metalView)) {
        ME::LogError("iOS: the app failed to initialize");
        app->Shutdown();
        return;
    }
    view.app = app;
    view.paused = NO;
}

- (void)setAppActive:(bool)active {
    if (app == nullptr) {
        return;
    }
    MEView* view = (MEView*)self.view;
    if (active) {
        app->Resume();
        view.paused = NO;
    } else {
        view.paused = YES;
        app->Pause();
    }
}

- (void)shutdownApp {
    MEView* view = (MEView*)self.view;
    view.paused = YES;
    view.app = nullptr;
    delete app;  // shuts the app down
    app = nullptr;
    delete metalView;
    metalView = nullptr;
}

- (void)dealloc {
    [self shutdownApp];
}

- (BOOL)prefersStatusBarHidden {
    return YES;
}

- (BOOL)prefersHomeIndicatorAutoHidden {
    return YES;
}

@end

// The window scene: creates the window, and tells the view controller when the app leaves or regains the foreground.
@interface MESceneDelegate : UIResponder <UIWindowSceneDelegate>
@property(nonatomic, strong) UIWindow* window;
@end

@implementation MESceneDelegate

- (void)scene:(UIScene*)scene
    willConnectToSession:(UISceneSession*)session
                 options:(UISceneConnectionOptions*)connectionOptions {
    if (![scene isKindOfClass:[UIWindowScene class]]) {
        return;
    }
    self.window = [[UIWindow alloc] initWithWindowScene:(UIWindowScene*)scene];
    self.window.rootViewController = [[MEViewController alloc] init];
    [self.window makeKeyAndVisible];
}

- (MEViewController*)gameController {
    return (MEViewController*)self.window.rootViewController;
}

- (void)sceneDidBecomeActive:(UIScene*)scene {
    [[self gameController] setAppActive:true];
}

// Calls, Control Center and going home all pass through here first; the app may be killed after it without notice.
- (void)sceneWillResignActive:(UIScene*)scene {
    [[self gameController] setAppActive:false];
}

- (void)sceneDidDisconnect:(UIScene*)scene {
    [[self gameController] shutdownApp];
}

@end

// The app: one scene, the delegate above.
@interface MEAppDelegate : UIResponder <UIApplicationDelegate>
@end

@implementation MEAppDelegate

- (UISceneConfiguration*)application:(UIApplication*)application
    configurationForConnectingSceneSession:(UISceneSession*)connectingSceneSession
                                   options:(UISceneConnectionOptions*)options {
    UISceneConfiguration* configuration = [[UISceneConfiguration alloc] initWithName:@"Default"
                                                                         sessionRole:connectingSceneSession.role];
    configuration.delegateClass = [MESceneDelegate class];
    return configuration;
}

@end

int main(int argc, char* argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([MEAppDelegate class]));
    }
}

#endif  // ME_IOS
