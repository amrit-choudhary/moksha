# Moksha_Cook: host tool that cooks res_list.json resources for one target platform (see cook.h).
# moksha_add_game runs it before each game build; cross-compiled builds use MOKSHA_COOK_EXECUTABLE instead.

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# stb_image (stb_image.h/.cpp, vendored, opaque) decodes PNGs for targets without texconv. Its warnings are not ours.
set(STB_IMAGE_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/stb_image.cpp")
if(MSVC)
    set_source_files_properties(${STB_IMAGE_SOURCE} PROPERTIES COMPILE_OPTIONS "/W0")
else()
    set_source_files_properties(${STB_IMAGE_SOURCE} PROPERTIES COMPILE_OPTIONS "-w")
endif()

add_executable(Moksha_Cook ${SOURCES})

target_link_libraries(Moksha_Cook PRIVATE Moksha_Shared)
