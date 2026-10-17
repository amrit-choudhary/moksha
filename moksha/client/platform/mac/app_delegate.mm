#ifdef ME_MAC

#import "app_delegate.h"
#import "view_delegate.h"

@interface MEAppDelegate ()

@end

@implementation MEAppDelegate

- (void)applicationWillFinishLaunching:(NSNotification*)notification {
    [self createMenuBar];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
}

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    CGRect screen = CGRectMake(100, 100, 1024, 768);
    NSRect screenNS = NSRectFromCGRect(screen);
    self.window = [[NSWindow alloc]
        initWithContentRect:screenNS
                  styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable)
                    backing:NSBackingStoreBuffered
                      defer:NO];
    [self.window setTitle:@"Moksha"];

    self.device = MTLCreateSystemDefaultDevice();
    if (!self.device) {
        NSLog(@"Metal is not supported on this device");
        return;
    }

    self.view = [[MEView alloc] initWithFrame:screenNS device:self.device];
    [self.view setColorPixelFormat:MTLPixelFormatBGRA8Unorm_sRGB];
    [self.view setClearColor:MTLClearColorMake(0.01, 0.01, 0.01, 1.0)];
    [self.view setDepthStencilPixelFormat:MTLPixelFormatDepth32Float];
    [self.view setClearDepth:1.0];

    MEViewDelegate* viewDelegate = [[MEViewDelegate alloc] initWithView:self.view device:self.device];
    [self.view setDelegate:viewDelegate];

    [self.view setPaused:NO];
    [self.view setEnableSetNeedsDisplay:NO];

    [self.window setContentView:self.view];
    [self.window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
}

- (void)applicationWillTerminate:(NSNotification*)aNotification {
    // Insert code here to tear down your application
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
    return YES;
}

- (void)createMenuBar {
    NSMenu* mainMenu = [[NSMenu alloc] init];

    NSMenuItem* appMenuItem = [[NSMenuItem alloc] init];
    [mainMenu addItem:appMenuItem];

    NSMenu* appMenu = [[NSMenu alloc] initWithTitle:@""];
    NSString* quitTitle = [@"Quit " stringByAppendingString:[[NSProcessInfo processInfo] processName]];
    NSMenuItem* quitMenuItem = [[NSMenuItem alloc] initWithTitle:quitTitle
                                                          action:@selector(terminate:)
                                                   keyEquivalent:@"q"];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];

    [NSApp setMainMenu:mainMenu];
}

@end

#endif  // ME_MAC
