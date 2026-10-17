# moksha_add_game: the one CMake call a game makes per executable.
#
#   moksha_add_game(<target> SOURCES <files...> RES_LIST <res_list.json> [HEADLESS])
#
# Creates <target> on the windowed client and <target>_cook, which cooks the engine's resources and then the
# game's (RES_LIST; its paths are relative to its folder) into resources/ next to the executable. HEADLESS also
# creates <target>_Headless on the headless client. Game includes resolve from the game's project root.
# The engine owns the per-platform target shape, so games never branch on the platform.

# The engine's resource list, cooked before every game's.
set(MOKSHA_ENGINE_RES_LIST "${CMAKE_CURRENT_LIST_DIR}/../resources/res_list.json" CACHE INTERNAL "")

# Tools the win-dx12 cook runs. Not found means the cook tries the plain names on PATH and reports a failure.
if(CMAKE_HOST_WIN32)
    find_program(MOKSHA_DXC dxc DOC "DirectX shader compiler used by Moksha_Cook")
    find_program(MOKSHA_TEXCONV texconv DOC "DirectXTex texconv used by Moksha_Cook")
endif()

# Cook target name for the windowed client of this platform; empty when there is none yet.
function(_moksha_windowed_cook_target outVar)
    set(name "")
    if(TARGET Moksha_Client AND CMAKE_SYSTEM_NAME STREQUAL "Windows" AND MOKSHA_RENDERER STREQUAL "DX12")
        set(name win-dx12)
    endif()
    set(${outVar} "${name}" PARENT_SCOPE)
endfunction()

# Adds <target>_cook, which <target> depends on.
function(_moksha_add_cook target cookTarget resList)
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

    add_custom_target(${target}_cook
        COMMAND "${cook}" --target ${cookTarget} --out "$<TARGET_FILE_DIR:${target}>/resources" ${toolArgs}
                "${MOKSHA_ENGINE_RES_LIST}" "${resList}"
        COMMENT "Cooking ${target} resources (${cookTarget})"
        VERBATIM)
    if(TARGET Moksha_Cook AND NOT MOKSHA_COOK_EXECUTABLE)
        add_dependencies(${target}_cook Moksha_Cook)
    endif()
    add_dependencies(${target} ${target}_cook)
endfunction()

function(moksha_add_game target)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "HEADLESS" "RES_LIST" "SOURCES")
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
        target_link_libraries(${target} PRIVATE Moksha_Entry Moksha_Client)
        _moksha_add_cook(${target} ${cookTarget} "${resList}")
    elseif(TARGET Moksha_Client)
        message(WARNING "moksha_add_game(${target}): no windowed game on '${CMAKE_SYSTEM_NAME}' "
                        "(${MOKSHA_RENDERER}) yet; only the headless one is created")
    endif()

    # Headless twin: same sources, no window, GPU or audio device; a console program on every desktop OS.
    if(ARG_HEADLESS)
        set(headless ${target}_Headless)
        add_executable(${headless} ${ARG_SOURCES})
        target_include_directories(${headless} PRIVATE "${PROJECT_SOURCE_DIR}")
        target_link_libraries(${headless} PRIVATE Moksha_EntryHeadless Moksha_ClientHeadless)
        _moksha_add_cook(${headless} headless "${resList}")
        # Both may cook into the same folder; one after the other, never at the same time.
        if(TARGET ${target}_cook)
            add_dependencies(${headless}_cook ${target}_cook)
        endif()
    endif()
endfunction()
