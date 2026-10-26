# Interface library for metal-cpp (Apple's headers, vendored unchanged) and the Apple frameworks the Metal client uses.
add_library(METAL_LIB INTERFACE)

target_include_directories(METAL_LIB INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/metal-cpp
)

# Metal, Foundation and QuartzCore for metal-cpp; AppKit and MetalKit for the macOS window and view (platform/mac).
target_link_libraries(METAL_LIB INTERFACE
    "-framework Metal"
    "-framework Foundation"
    "-framework QuartzCore"
    "-framework AppKit"
    "-framework MetalKit"
)