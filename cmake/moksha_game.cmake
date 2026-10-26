# moksha_add_game / moksha_add_server: the one CMake call a game makes per executable.
#
#   moksha_add_game(<target> SOURCES <files...> RES_LIST <res_list.json> [LIBRARIES <libs...>] [HEADLESS])
#   moksha_add_server(<target> SOURCES <files...> [RES_LIST <res_list.json>] [LIBRARIES <libs...>])
#
# Creates <target> on the windowed client and <target>_cook, which cooks the engine's resources and then the
# game's (RES_LIST; its paths are relative to its folder) into resources/ next to the executable. HEADLESS also
# creates <target>_Headless on the headless client. moksha_add_server creates a console program on the server
# module; its RES_LIST (data only: config, game data) is copied next to it. Includes resolve from the project root.
# LIBRARIES are the game's own libraries, linked into every executable created; pass them here rather than calling
# target_link_libraries, since a platform may create no windowed target. The engine owns the per-platform target
# shape, so games never branch on the platform.

# The engine's resource list, cooked before every game's.
set(MOKSHA_ENGINE_RES_LIST "${CMAKE_CURRENT_LIST_DIR}/../resources/res_list.json" CACHE INTERNAL "")

# Tools the win-dx12 cook runs. Not found means the cook tries the plain names on PATH and reports a failure.
if(CMAKE_HOST_WIN32)
    find_program(MOKSHA_DXC dxc DOC "DirectX shader compiler used by Moksha_Cook")
    find_program(MOKSHA_TEXCONV texconv DOC "DirectXTex texconv used by Moksha_Cook")
endif()

# The mac-metal cook compiles shaders with `xcrun metal`, which needs Xcode and its Metal toolchain. xcode-select may
# point at the Command Line Tools (no Metal compiler), so fall back to Xcode's folder; set this to override.
if(CMAKE_HOST_APPLE)
    set(MOKSHA_METAL_DEVELOPER_DIR "" CACHE PATH "Developer folder whose xcrun finds the Metal compiler (empty: detect)")
    set(_mokshaMetalDeveloperDir "${MOKSHA_METAL_DEVELOPER_DIR}")
    if(NOT _mokshaMetalDeveloperDir)
        execute_process(COMMAND xcrun -sdk macosx -f metal RESULT_VARIABLE _mokshaMetalFound OUTPUT_QUIET ERROR_QUIET)
        set(_mokshaXcodeDir "/Applications/Xcode.app/Contents/Developer")
        if(NOT _mokshaMetalFound EQUAL 0 AND EXISTS "${_mokshaXcodeDir}")
            execute_process(COMMAND ${CMAKE_COMMAND} -E env DEVELOPER_DIR=${_mokshaXcodeDir} xcrun -sdk macosx -f metal
                            RESULT_VARIABLE _mokshaMetalFound OUTPUT_QUIET ERROR_QUIET)
            if(_mokshaMetalFound EQUAL 0)
                set(_mokshaMetalDeveloperDir "${_mokshaXcodeDir}")
            endif()
        endif()
        if(NOT _mokshaMetalFound EQUAL 0)
            message(WARNING "Moksha: no Metal compiler found; install Xcode and its Metal toolchain (Xcode > Settings > "
                            "Components), or set MOKSHA_METAL_DEVELOPER_DIR. mac-metal shader cooking will fail.")
        endif()
    endif()
    set(MOKSHA_METAL_DEVELOPER_DIR_FOUND "${_mokshaMetalDeveloperDir}" CACHE INTERNAL "")
endif()

# Cook target name for the windowed client of this platform; empty when there is none yet.
function(_moksha_windowed_cook_target outVar)
    set(name "")
    if(TARGET Moksha_Client AND CMAKE_SYSTEM_NAME STREQUAL "Windows" AND MOKSHA_RENDERER STREQUAL "DX12")
        set(name win-dx12)
    elseif(TARGET Moksha_Client AND CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND MOKSHA_RENDERER STREQUAL "METAL")
        set(name mac-metal)
    endif()
    set(${outVar} "${name}" PARENT_SCOPE)
endfunction()

# Adds <target>_cook, which <target> depends on. engineList is the engine's list, or empty for none.
function(_moksha_add_cook target cookTarget engineList resList)
    if(MOKSHA_COOK_EXECUTABLE)
        set(cook "${MOKSHA_COOK_EXECUTABLE}")
    elseif(TARGET Moksha_Cook)
        set(cook "$<TARGET_FILE:Moksha_Cook>")
    else()
        message(FATAL_ERROR "moksha_add_game: no cook tool. When cross-compiling, set MOKSHA_COOK_EXECUTABLE to a "
                            "Moksha_Cook built for the host.")
    endif()

    set(toolArgs "")
    if(MOKSHA_DXC)
        list(APPEND toolArgs --dxc "${MOKSHA_DXC}")
    endif()
    if(MOKSHA_TEXCONV)
        list(APPEND toolArgs --texconv "${MOKSHA_TEXCONV}")
    endif()
    if(MOKSHA_METAL_DEVELOPER_DIR_FOUND)
        list(APPEND toolArgs --metal-developer-dir "${MOKSHA_METAL_DEVELOPER_DIR_FOUND}")
    endif()

    add_custom_target(${target}_cook
        COMMAND "${cook}" --target ${cookTarget} --out "$<TARGET_FILE_DIR:${target}>/resources" ${toolArgs}
                ${engineList} "${resList}"
        COMMENT "Cooking ${target} resources (${cookTarget})"
        VERBATIM)
    if(TARGET Moksha_Cook AND NOT MOKSHA_COOK_EXECUTABLE)
        add_dependencies(${target}_cook Moksha_Cook)
    endif()
    add_dependencies(${target} ${target}_cook)
endfunction()

function(moksha_add_game target)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "HEADLESS" "RES_LIST" "SOURCES;LIBRARIES")
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "moksha_add_game(${target}): unknown arguments: ${ARG_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "moksha_add_game(${target}): SOURCES is required")
    endif()
    if(NOT ARG_RES_LIST)
        message(FATAL_ERROR "moksha_add_game(${target}): RES_LIST is required")
    endif()
    get_filename_component(resList "${ARG_RES_LIST}" ABSOLUTE)
    if(NOT EXISTS "${resList}")
        message(FATAL_ERROR "moksha_add_game(${target}): RES_LIST not found: ${resList}")
    endif()

    # Windowed game.
    _moksha_windowed_cook_target(cookTarget)
    if(cookTarget)
        add_executable(${target} WIN32 ${ARG_SOURCES})
        target_include_directories(${target} PRIVATE "${PROJECT_SOURCE_DIR}")
        target_link_libraries(${target} PRIVATE Moksha_Entry Moksha_Client ${ARG_LIBRARIES})
        _moksha_add_cook(${target} ${cookTarget} "${MOKSHA_ENGINE_RES_LIST}" "${resList}")
    elseif(TARGET Moksha_Client)
        message(WARNING "moksha_add_game(${target}): no windowed game on '${CMAKE_SYSTEM_NAME}' "
                        "(${MOKSHA_RENDERER}) yet; only the headless one is created")
    endif()

    # Headless twin: same sources, no window, GPU or audio device; a console program on every desktop OS.
    if(ARG_HEADLESS)
        set(headless ${target}_Headless)
        add_executable(${headless} ${ARG_SOURCES})
        target_include_directories(${headless} PRIVATE "${PROJECT_SOURCE_DIR}")
        target_link_libraries(${headless} PRIVATE Moksha_EntryHeadless Moksha_ClientHeadless ${ARG_LIBRARIES})
        _moksha_add_cook(${headless} headless "${MOKSHA_ENGINE_RES_LIST}" "${resList}")
        # Both may cook into the same folder; one after the other, never at the same time.
        if(TARGET ${target}_cook)
            add_dependencies(${headless}_cook ${target}_cook)
        endif()
    endif()
endfunction()

function(moksha_add_server target)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "" "RES_LIST" "SOURCES;LIBRARIES")
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "moksha_add_server(${target}): unknown arguments: ${ARG_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "moksha_add_server(${target}): SOURCES is required")
    endif()

    add_executable(${target} ${ARG_SOURCES})
    target_include_directories(${target} PRIVATE "${PROJECT_SOURCE_DIR}")
    target_link_libraries(${target} PRIVATE Moksha_Server ${ARG_LIBRARIES})

    # Servers need no engine resources, only their own data.
    if(ARG_RES_LIST)
        get_filename_component(resList "${ARG_RES_LIST}" ABSOLUTE)
        if(NOT EXISTS "${resList}")
            message(FATAL_ERROR "moksha_add_server(${target}): RES_LIST not found: ${resList}")
        endif()
        _moksha_add_cook(${target} headless "" "${resList}")
    endif()
endfunction()
