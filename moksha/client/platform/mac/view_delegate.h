/**
 * Gets update and draw events from the Mac Framework
 */

#ifdef ME_MAC
#ifdef __OBJC__

#pragma once

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "moksha/client/main/main_mac.h"

@interface MEViewDelegate : NSObject <MTKViewDelegate>

@property(nonatomic, nonnull, assign) MTKView *view;
@property(nonatomic, nonnull, assign) id<MTLDevice> device;
@property(nonatomic, nonnull, assign) ME::ClientApp *gameMain;

- (nonnull instancetype)initWithView:(nonnull MTKView *)view device:(nonnull id<MTLDevice>)device;

@end

#endif  // __OBJC__
#endif  // ME_MAC
