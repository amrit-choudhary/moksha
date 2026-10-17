/**
 * Subclass of MTKView for Metal rendering on macOS.
 * Subclassing to get input events and input handling.
 */

#ifdef ME_MAC
#ifdef __OBJC__

#pragma once

#import <MetalKit/MetalKit.h>

@interface MEView : MTKView
@end

#endif  // __OBJC__
#endif  // ME_MAC
