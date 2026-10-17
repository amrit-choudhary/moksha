#ifdef ME_MAC

#import "view_delegate.h"
#import <iostream>

@interface MEViewDelegate ()

@end

@implementation MEViewDelegate

- (nonnull instancetype)initWithView:(nonnull MTKView*)view device:(nonnull id<MTLDevice>)device {
    self = [super init];
    if (self) {
        self.view = view;
        self.device = device;

        // Bridge the Objective-C Metal handles to the metal-cpp types ClientApp expects.
        MTL::Device* mtlDevice = reinterpret_cast<MTL::Device*>((__bridge void*)device);
        MTK::View* mtkView = reinterpret_cast<MTK::View*>((__bridge void*)view);

        self.gameMain = new ME::ClientApp();
        self.gameMain->Init(mtlDevice, mtkView);
    }
    return self;
}

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size {
    /// Respond to drawable size or orientation changes here
}

- (void)drawInMTKView:(nonnull MTKView*)view {
    self.gameMain->Update();
    // std::cout << "Drawing frame..." << std::endl;
}

@end

#endif  // ME_MAC
