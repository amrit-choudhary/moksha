# CMakeLists.txt for miniaudio library

set(MINIAUDIO_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/miniaudio.c"
)

add_library(miniaudio STATIC ${MINIAUDIO_SRCS})

target_include_directories(miniaudio PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Ensure the implementation is compiled as C
set_target_properties(miniaudio PROPERTIES
    LINKER_LANGUAGE C
)

# Platform-specific system libraries miniaudio may need
if (WIN32)
    target_compile_definitions(miniaudio PUBLIC
        MA_ENABLE_ONLY_SPECIFIC_BACKENDS
        MA_ENABLE_WASAPI=1
        MA_NO_DSOUND
        MA_NO_WINMM
    )
    # Make system libs PUBLIC so linking is satisfied for consumers.
    target_link_libraries(miniaudio PUBLIC ole32 avrt)
elseif(APPLE)
    target_link_libraries(miniaudio PRIVATE "-framework CoreAudio" "-framework AudioUnit" "-framework AudioToolbox" "-framework CoreServices")
else() # Linux/Unix
    target_link_libraries(miniaudio PRIVATE pthread dl asound m)
endif()

# Optional: expose compile defs for miniaudio customization here, e.g.:
# target_compile_definitions(miniaudio PUBLIC MINIAUDIO_NO_STDIO)
