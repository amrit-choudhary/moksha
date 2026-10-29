# Interface library for metal-cpp (Apple's headers, vendored unchanged) and the Apple frameworks the Metal client uses.
add_library(METAL_LIB INTERFACE)

target_include_directories(METAL_LIB INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/metal-cpp
)

# Metal, Foundation and QuartzCore for metal-cpp; MetalKit for the view; the OS's UI framework for the window
# (AppKit for platform/mac, UIKit for platform/ios).
target_link_libraries(METAL_LIB INTERFACE
    "-framework Metal"
    "-framework Foundation"
    "-framework QuartzCore"
    "-framework MetalKit"
)
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_link_libraries(METAL_LIB INTERFACE "-framework UIKit")
else()
    target_link_libraries(METAL_LIB INTERFACE "-framework AppKit")
endif()
