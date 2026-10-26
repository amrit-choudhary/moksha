#ifdef ME_METAL

/**
 * metal-cpp's implementation (selectors and class lookups). It must be compiled into exactly one file of the
 * program; every other file includes the metal-cpp headers without these defines.
 */

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#endif  // ME_METAL
