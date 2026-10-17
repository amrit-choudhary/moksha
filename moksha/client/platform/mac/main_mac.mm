/**
 * Main entry point for the macOS application.
 * This file is specific to macOS and uses Objective-C for application management.
 */

#ifdef ME_MAC

#import "app_delegate.h"

#import <Cocoa/Cocoa.h>

#include <iostream>
#include "moksha/shared/misc/utils.h"

int main(int argc, char *argv[]) {
    @autoreleasepool {
        ME::SetPaths(argv[0], argv[1]);
        NSApplication *app = [NSApplication sharedApplication];
        MEAppDelegate *delegate = [[NSClassFromString(@"MEAppDelegate") alloc] init];
        [app setDelegate:delegate];
        [app run];
    }
    return 0;
}

#endif  // ME_MAC
