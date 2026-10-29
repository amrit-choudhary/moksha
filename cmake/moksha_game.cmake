# moksha_add_game / moksha_add_server: the one CMake call a game makes per executable.
#
#   moksha_add_game(<target> SOURCES <files...> RES_LIST <res_list.json> [LIBRARIES <libs...>] [HEADLESS]
#                   [DISPLAY_NAME <name>] [BUNDLE_ID <id>] [ORIENTATION PORTRAIT|LANDSCAPE|ANY])
#   moksha_add_server(<target> SOURCES <files...> [RES_LIST <res_list.json>] [LIBRARIES <libs...>])
#
# Creates <target> on the windowed client and <target>_cook, which cooks the engine's resources and then the
# game's (RES_LIST; its paths are relative to its folder) into resources/ next to the executable. On iOS <target> is
# an app bundle instead: resources cook into data/ inside it, and DISPLAY_NAME (default: the target), BUNDLE_ID
# (default: MOKSHA_BUNDLE_ID_PREFIX + the target) and ORIENTATION (default ANY) fill its Info.plist. HEADLESS also
# creates <target>_Headless on the headless client (not on iOS). moksha_add_server creates a console program on the server
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

# The mac-metal and ios-metal cooks compile shaders with `xcrun metal`, which needs Xcode and its Metal toolchain.
# xcode-select may point at the Command Line Tools (no Metal compiler), so fall back to Xcode's folder; set this to
# override.
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
                            "Components), or set MOKSHA_METAL_DEVELOPER_DIR. Metal shader cooking will fail.")
        endif()
    endif()
    set(MOKSHA_METAL_DEVELOPER_DIR_FOUND "${_mokshaMetalDeveloperDir}" CACHE INTERNAL "")
endif()

# iOS games are app bundles built with the Xcode generator; the signing team comes from
# CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM (a per-machine preset).
set(MOKSHA_IOS_PLIST_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/ios/Info.plist.in" CACHE INTERNAL "")
set(MOKSHA_BUNDLE_ID_PREFIX "" CACHE STRING "Reverse-DNS prefix of iOS bundle ids, e.g. com.yourname.games")

# Cook target name for the windowed client of this platform; empty when there is none yet.
function(_moksha_windowed_cook_target outVar)
    set(name "")
    if(TARGET Moksha_Client AND CMAKE_SYSTEM_NAME STREQUAL "Windows" AND MOKSHA_RENDERER STREQUAL "DX12")
        set(name win-dx12)
    elseif(TARGET Moksha_Client AND CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND MOKSHA_RENDERER STREQUAL "METAL")
        set(name mac-metal)
    elseif(TARGET Moksha_Client AND CMAKE_SYSTEM_NAME STREQUAL "iOS" AND MOKSHA_RENDERER STREQUAL "METAL")
        set(name ios-metal)
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

    # iOS cooks into a staging folder that a post-build step copies into the bundle as data/: Xcode's bundle path holds
    # a ${EFFECTIVE_PLATFORM_NAME} that only its own build settings expand, and an iOS bundle can't hold a top-level
    # resources/ folder (the bundle then fails to install).
    if(cookTarget STREQUAL "ios-metal")
        set(outDir "${CMAKE_CURRENT_BINARY_DIR}/${target}_data")
    else()
        set(outDir "$<TARGET_FILE_DIR:${target}>/resources")
    endif()

    add_custom_target(${target}_cook
        COMMAND "${cook}" --target ${cookTarget} --out "${outDir}" ${toolArgs}
                ${engineList} "${resList}"
        COMMENT "Cooking ${target} resources (${cookTarget})"
        VERBATIM)
    if(TARGET Moksha_Cook AND NOT MOKSHA_COOK_EXECUTABLE)
        add_dependencies(${target}_cook Moksha_Cook)
    endif()
    add_dependencies(${target} ${target}_cook)

    # Not VERBATIM, so Xcode expands the bundle path. Runs before Xcode signs the bundle, which seals data/ in.
    if(cookTarget STREQUAL "ios-metal")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${outDir}" "$<TARGET_BUNDLE_CONTENT_DIR:${target}>/data")
    endif()
endfunction()

# Makes target an iOS app: an Info.plist from the engine's template, the bundle id and the display name. iPhone only
# for now.
function(_moksha_ios_bundle target bundleId displayName orientation)
    if(NOT bundleId)
        if(NOT MOKSHA_BUNDLE_ID_PREFIX)
            message(FATAL_ERROR "moksha_add_game(${target}): iOS needs a bundle id; pass BUNDLE_ID or set "
                                "MOKSHA_BUNDLE_ID_PREFIX")
        endif()
        string(TOLOWER "${target}" idSuffix)
        string(REGEX REPLACE "[^a-z0-9.-]" "-" idSuffix "${idSuffix}")
        set(bundleId "${MOKSHA_BUNDLE_ID_PREFIX}.${idSuffix}")
    endif()
    if(NOT displayName)
        set(displayName "${target}")
    endif()

    if(NOT orientation OR orientation STREQUAL "ANY")
        set(orientations UIInterfaceOrientationPortrait UIInterfaceOrientationLandscapeLeft
                         UIInterfaceOrientationLandscapeRight)
    elseif(orientation STREQUAL "PORTRAIT")
        set(orientations UIInterfaceOrientationPortrait)
    elseif(orientation STREQUAL "LANDSCAPE")
        set(orientations UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight)
    else()
        message(FATAL_ERROR "moksha_add_game(${target}): ORIENTATION must be PORTRAIT, LANDSCAPE or ANY")
    endif()

    # Values the template reads (@VAR@).
    set(MOKSHA_BUNDLE_EXECUTABLE "${target}")
    set(MOKSHA_BUNDLE_ID "${bundleId}")
    set(MOKSHA_BUNDLE_DISPLAY_NAME "${displayName}")
    set(MOKSHA_BUNDLE_VERSION "1.0")
    set(MOKSHA_BUNDLE_BUILD "1")
    set(MOKSHA_BUNDLE_ORIENTATIONS "")
    foreach(entry IN LISTS orientations)
        string(APPEND MOKSHA_BUNDLE_ORIENTATIONS "        <string>${entry}</string>\n")
    endforeach()
    set(plist "${CMAKE_CURRENT_BINARY_DIR}/${target}_Info.plist")
    configure_file("${MOKSHA_IOS_PLIST_TEMPLATE}" "${plist}" @ONLY)

    set_target_properties(${target} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${plist}"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${bundleId}"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1"
        XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic")
endfunction()

function(moksha_add_game target)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "HEADLESS" "RES_LIST;DISPLAY_NAME;BUNDLE_ID;ORIENTATION" "SOURCES;LIBRARIES")
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
    if(cookTarget STREQUAL "ios-metal")
        add_executable(${target} MACOSX_BUNDLE ${ARG_SOURCES})
        _moksha_ios_bundle(${target} "${ARG_BUNDLE_ID}" "${ARG_DISPLAY_NAME}" "${ARG_ORIENTATION}")
    elseif(cookTarget)
        add_executable(${target} WIN32 ${ARG_SOURCES})
    endif()
    if(cookTarget)
        target_include_directories(${target} PRIVATE "${PROJECT_SOURCE_DIR}")
        target_link_libraries(${target} PRIVATE Moksha_Entry Moksha_Client ${ARG_LIBRARIES})
        _moksha_add_cook(${target} ${cookTarget} "${MOKSHA_ENGINE_RES_LIST}" "${resList}")
    elseif(TARGET Moksha_Client)
        message(WARNING "moksha_add_game(${target}): no windowed game on '${CMAKE_SYSTEM_NAME}' "
                        "(${MOKSHA_RENDERER}) yet; only the headless one is created")
    endif()

    # Headless twin: same sources, no window, GPU or audio device; a console program on every desktop OS. iOS runs
    # only app bundles.
    if(ARG_HEADLESS AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
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
    # Servers are console programs; iOS runs only app bundles.
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        return()
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
