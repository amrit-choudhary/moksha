#ifdef ME_MAC
#ifdef __OBJC__

#pragma once

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#import "metal_view.h"

@interface MEAppDelegate : NSObject <NSApplicationDelegate>

@property(assign) IBOutlet NSWindow *window;
@property(nonatomic, assign) MTKView *view;
@property(nonatomic, assign) id<MTLDevice> device;

- (void)createMenuBar;

@end

#endif  // __OBJC__
#endif  // ME_MAC
