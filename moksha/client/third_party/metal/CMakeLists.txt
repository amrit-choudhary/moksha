# Define an interface library for metal-cpp and its extensions
add_library(METAL_LIB INTERFACE)

# Specify the include directories relative to this CMakeLists.txt
target_include_directories(METAL_LIB INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/metal-cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/metal-cpp-extensions
)

# Link the required Apple frameworks
target_link_libraries(METAL_LIB INTERFACE
    "-framework Metal"
    "-framework Foundation"
    "-framework QuartzCore"
    "-framework AppKit"
    "-framework MetalKit"
)