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
# creates <target>_Headless on the headless client (not on iOS or Android). On web <target> is a page: an .html, .js,
# .wasm and .data, with the resources packed into the .data, and DISPLAY_NAME as its title. On Android <target> is
# lib<target>.so, packed after each build into a signed debug <target>.apk with the resources as its assets/;
# DISPLAY_NAME, BUNDLE_ID (the package name) and ORIENTATION work as on iOS, and <target>_install installs and starts
# it on the connected device. moksha_add_server creates a console program on the server module (not on iOS, web or
# Android); its RES_LIST (data only: config, game data) is copied next to it. Includes resolve from the project root.
# LIBRARIES are the game's own libraries, linked into every executable created; pass them here rather than calling
# target_link_libraries, since a platform may create no windowed target. The engine owns the per-platform target
# shape, so games never branch on the platform.

# The engine's resource list, cooked before every game's.
set(MOKSHA_ENGINE_RES_LIST "${CMAKE_CURRENT_LIST_DIR}/../resources/res_list.json" CACHE INTERNAL "")

# The tool the win-dx12 cook runs. Not found means the cook tries the plain name on PATH and reports a failure.
if(CMAKE_HOST_WIN32)
    find_program(MOKSHA_DXC dxc DOC "DirectX shader compiler used by Moksha_Cook")
endif()

# The tool the win-vulkan and android-vulkan cooks run to compile glsl to SPIR-V: the Vulkan SDK's, or the NDK's
# (shader-tools/<host>). Not found means the plain name on PATH.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND MOKSHA_RENDERER STREQUAL "VULKAN")
    find_program(MOKSHA_GLSLC glslc HINTS "$ENV{VULKAN_SDK}/Bin" DOC "GLSL to SPIR-V compiler used by Moksha_Cook")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    file(GLOB _mokshaNdkShaderTools LIST_DIRECTORIES true "${ANDROID_NDK}/shader-tools/*")
    find_program(MOKSHA_GLSLC glslc HINTS ${_mokshaNdkShaderTools} NO_DEFAULT_PATH
                 DOC "GLSL to SPIR-V compiler used by Moksha_Cook")
endif()

# Android packaging tools: SDK (ANDROID_HOME) build-tools and platform, newest installed unless set; JDK tools from
# JAVA_HOME or PATH. Checked by the first moksha_add_game, so engine-only builds don't need them.
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    file(TO_CMAKE_PATH "$ENV{ANDROID_HOME}" _mokshaAndroidHome)
    set(MOKSHA_ANDROID_SDK "${_mokshaAndroidHome}" CACHE PATH "Android SDK folder (default: ANDROID_HOME)")

    file(GLOB _mokshaBuildTools LIST_DIRECTORIES true "${MOKSHA_ANDROID_SDK}/build-tools/*")
    list(SORT _mokshaBuildTools COMPARE NATURAL)
    list(POP_BACK _mokshaBuildTools _mokshaBuildToolsNewest)
    set(MOKSHA_ANDROID_BUILD_TOOLS "${_mokshaBuildToolsNewest}" CACHE PATH "Android SDK build-tools folder to package with")

    file(GLOB _mokshaAndroidJars "${MOKSHA_ANDROID_SDK}/platforms/android-*/android.jar")
    list(SORT _mokshaAndroidJars COMPARE NATURAL)
    list(POP_BACK _mokshaAndroidJars _mokshaAndroidJarNewest)
    set(MOKSHA_ANDROID_JAR "${_mokshaAndroidJarNewest}" CACHE FILEPATH
        "android.jar to package against; its platform is the apps' target SDK")

    find_program(MOKSHA_AAPT2 aapt2 HINTS "${MOKSHA_ANDROID_BUILD_TOOLS}" NO_DEFAULT_PATH)
    find_program(MOKSHA_ZIPALIGN zipalign HINTS "${MOKSHA_ANDROID_BUILD_TOOLS}" NO_DEFAULT_PATH)
    find_program(MOKSHA_APKSIGNER NAMES apksigner.bat apksigner HINTS "${MOKSHA_ANDROID_BUILD_TOOLS}" NO_DEFAULT_PATH)
    find_program(MOKSHA_ADB adb HINTS "${MOKSHA_ANDROID_SDK}/platform-tools")
    find_program(MOKSHA_JAR jar HINTS "$ENV{JAVA_HOME}/bin")
    find_program(MOKSHA_KEYTOOL keytool HINTS "$ENV{JAVA_HOME}/bin")
endif()

set(MOKSHA_ANDROID_MANIFEST_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/android/AndroidManifest.xml.in" CACHE INTERNAL "")

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

set(MOKSHA_WEB_SHELL_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/web/shell.html.in" CACHE INTERNAL "")

# Cook target name for the windowed client of this platform; empty when there is none yet.
function(_moksha_windowed_cook_target outVar)
    set(name "")
    if(TARGET Moksha_Client AND CMAKE_SYSTEM_NAME STREQUAL "Windows" AND MOKSHA_RENDERER STREQUAL "DX12")
        set(name win-dx12)
    elseif(TARGET Moksha_Client AND CMAKE_SYSTEM_NAME STREQUAL "Windows" AND MOKSHA_RENDERER STREQUAL "VULKAN")
        set(name win-vulkan)
    elseif(TARGET Moksha_Client AND CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND MOKSHA_RENDERER STREQUAL "METAL")
        set(name mac-metal)
    elseif(TARGET Moksha_Client AND CMAKE_SYSTEM_NAME STREQUAL "iOS" AND MOKSHA_RENDERER STREQUAL "METAL")
        set(name ios-metal)
    elseif(TARGET Moksha_Client AND CMAKE_SYSTEM_NAME STREQUAL "Emscripten" AND MOKSHA_RENDERER STREQUAL "WEBGPU")
        set(name web-webgpu)
    elseif(TARGET Moksha_Client AND CMAKE_SYSTEM_NAME STREQUAL "Android" AND MOKSHA_RENDERER STREQUAL "VULKAN")
        set(name android-vulkan)
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
    if(MOKSHA_GLSLC)
        list(APPEND toolArgs --glslc "${MOKSHA_GLSLC}")
    endif()
    if(MOKSHA_METAL_DEVELOPER_DIR_FOUND)
        list(APPEND toolArgs --metal-developer-dir "${MOKSHA_METAL_DEVELOPER_DIR_FOUND}")
    endif()

    # iOS cooks into a staging folder that a post-build step copies into the bundle as data/: Xcode's bundle path holds
    # a ${EFFECTIVE_PLATFORM_NAME} that only its own build settings expand, and an iOS bundle can't hold a top-level
    # resources/ folder (the bundle then fails to install).
    if(cookTarget STREQUAL "ios-metal")
        set(outDir "${CMAKE_CURRENT_BINARY_DIR}/${target}_data")
    elseif(cookTarget STREQUAL "android-vulkan")
        # The APK's assets/ folder, packed by _moksha_android_apk.
        set(outDir "${CMAKE_CURRENT_BINARY_DIR}/${target}_apk/assets")
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

# The app id (iOS bundle id, Android package name): BUNDLE_ID, or MOKSHA_BUNDLE_ID_PREFIX + the lowercased target with
# every character outside [a-z0-9.] replaced by replaceChar ("-" on iOS; Android allows only "_").
function(_moksha_app_id outVar target bundleId replaceChar)
    if(NOT bundleId)
        if(NOT MOKSHA_BUNDLE_ID_PREFIX)
            message(FATAL_ERROR "moksha_add_game(${target}): ${CMAKE_SYSTEM_NAME} needs an app id; pass BUNDLE_ID or "
                                "set MOKSHA_BUNDLE_ID_PREFIX")
        endif()
        string(TOLOWER "${target}" idSuffix)
        string(REGEX REPLACE "[^a-z0-9.]" "${replaceChar}" idSuffix "${idSuffix}")
        set(bundleId "${MOKSHA_BUNDLE_ID_PREFIX}.${idSuffix}")
    endif()
    set(${outVar} "${bundleId}" PARENT_SCOPE)
endfunction()

# Makes target an iOS app: an Info.plist from the engine's template, the bundle id and the display name. iPhone only
# for now.
function(_moksha_ios_bundle target bundleId displayName orientation)
    _moksha_app_id(bundleId ${target} "${bundleId}" "-")
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

# Relinks target after every cook, so a step that packs the cooked resources at link time (or after it) always sees
# the latest ones. Returns the stamp file target's link depends on.
function(_moksha_relink_after_cook outVar target)
    set(stamp "${CMAKE_CURRENT_BINARY_DIR}/${target}_cook.stamp")
    file(TOUCH "${stamp}")
    add_custom_command(TARGET ${target}_cook POST_BUILD COMMAND ${CMAKE_COMMAND} -E touch "${stamp}")
    set(${outVar} "${stamp}" PARENT_SCOPE)
endfunction()

# Makes target a web page: an .html from the engine's template, with the cooked resources packed in.
function(_moksha_web_page target displayName)
    if(NOT displayName)
        set(displayName "${target}")
    endif()
    set(MOKSHA_WEB_TITLE "${displayName}")
    set(shell "${CMAKE_CURRENT_BINARY_DIR}/${target}_shell.html")
    configure_file("${MOKSHA_WEB_SHELL_TEMPLATE}" "${shell}" @ONLY)

    _moksha_relink_after_cook(stamp ${target})
    set_target_properties(${target} PROPERTIES SUFFIX ".html" LINK_DEPENDS "${shell};${stamp}")
    target_link_options(${target} PRIVATE
        -sENVIRONMENT=web -sALLOW_MEMORY_GROWTH=1 -sSTACK_SIZE=1048576 -lidbfs.js
        "--shell-file=${shell}"
        "SHELL:--preload-file ${CMAKE_CURRENT_BINARY_DIR}/resources@/resources")
endfunction()

# The debug signing key every Android game in this build shares, created once by keytool in the build folder. A new
# build folder makes a new key, and Android then refuses to update an app signed with the old one (uninstall it first).
function(_moksha_android_debug_keystore outVar)
    set(keystore "${CMAKE_BINARY_DIR}/moksha_debug.keystore")
    if(NOT TARGET Moksha_AndroidDebugKeystore)
        add_custom_command(OUTPUT "${keystore}"
            COMMAND "${MOKSHA_KEYTOOL}" -genkeypair -noprompt -keystore "${keystore}" -storepass android
                    -keypass android -alias androiddebugkey -keyalg RSA -keysize 2048 -validity 10000
                    -dname "CN=Android Debug,O=Android,C=US"
            COMMENT "Creating the Android debug keystore"
            VERBATIM)
        add_custom_target(Moksha_AndroidDebugKeystore DEPENDS "${keystore}")
    endif()
    set(${outVar} "${keystore}" PARENT_SCOPE)
endfunction()

# Packs target (lib<target>.so) and its cooked assets/ into <target>.apk after each link: aapt2 links the manifest
# against android.jar and adds assets/, jar adds lib/<abi>/ uncompressed, zipalign aligns it (.so files to 16 KB
# pages), apksigner signs it with the debug key. Adds <target>_install (adb install, then start the activity).
function(_moksha_android_apk target appId displayName orientation)
    foreach(tool MOKSHA_ANDROID_JAR MOKSHA_AAPT2 MOKSHA_ZIPALIGN MOKSHA_APKSIGNER MOKSHA_ADB MOKSHA_JAR MOKSHA_KEYTOOL)
        if(NOT ${tool} OR NOT EXISTS "${${tool}}")
            message(FATAL_ERROR "moksha_add_game(${target}): ${tool} not found ('${${tool}}'). Set ANDROID_HOME (with "
                                "build-tools, a platform and platform-tools installed) and JAVA_HOME, or set ${tool}.")
        endif()
    endforeach()

    if(NOT displayName)
        set(displayName "${target}")
    endif()
    if(NOT orientation OR orientation STREQUAL "ANY")
        set(MOKSHA_ANDROID_ORIENTATION "unspecified")
    elseif(orientation STREQUAL "PORTRAIT")
        set(MOKSHA_ANDROID_ORIENTATION "portrait")
    elseif(orientation STREQUAL "LANDSCAPE")
        set(MOKSHA_ANDROID_ORIENTATION "sensorLandscape")
    else()
        message(FATAL_ERROR "moksha_add_game(${target}): ORIENTATION must be PORTRAIT, LANDSCAPE or ANY")
    endif()

    # Target SDK: the API level of the platform packaged against (platforms/android-<level>[.<minor>]/android.jar).
    if(NOT MOKSHA_ANDROID_JAR MATCHES "android-([0-9]+)[^/]*/android\\.jar$")
        message(FATAL_ERROR "moksha_add_game(${target}): can't read the API level from ${MOKSHA_ANDROID_JAR}")
    endif()
    set(MOKSHA_ANDROID_TARGET_SDK "${CMAKE_MATCH_1}")

    # Values the template reads (@VAR@).
    set(MOKSHA_ANDROID_PACKAGE "${appId}")
    set(MOKSHA_ANDROID_LIB "${target}")
    set(MOKSHA_ANDROID_DISPLAY_NAME "${displayName}")
    set(MOKSHA_ANDROID_VERSION_CODE "1")
    set(MOKSHA_ANDROID_VERSION_NAME "1.0")
    set(MOKSHA_ANDROID_MIN_SDK "${ANDROID_PLATFORM_LEVEL}")
    set(manifest "${CMAKE_CURRENT_BINARY_DIR}/${target}_AndroidManifest.xml")
    configure_file("${MOKSHA_ANDROID_MANIFEST_TEMPLATE}" "${manifest}" @ONLY)

    _moksha_android_debug_keystore(keystore)
    add_dependencies(${target} Moksha_AndroidDebugKeystore)
    _moksha_relink_after_cook(stamp ${target})
    set_target_properties(${target} PROPERTIES LINK_DEPENDS "${manifest};${stamp}")

    set(apkDir "${CMAKE_CURRENT_BINARY_DIR}/${target}_apk")
    set(libDir "${apkDir}/lib/${ANDROID_ABI}")
    set(apk "${CMAKE_CURRENT_BINARY_DIR}/${target}.apk")
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${libDir}"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${target}>" "${libDir}"
        COMMAND "${MOKSHA_AAPT2}" link -o "${apkDir}/unaligned.apk" --manifest "${manifest}" -I "${MOKSHA_ANDROID_JAR}"
                -A "${apkDir}/assets" $<$<CONFIG:Debug>:--debug-mode>
        COMMAND "${MOKSHA_JAR}" --update --no-manifest --no-compress "--file=${apkDir}/unaligned.apk" -C "${apkDir}" lib
        COMMAND "${MOKSHA_ZIPALIGN}" -f -P 16 4 "${apkDir}/unaligned.apk" "${apkDir}/aligned.apk"
        COMMAND "${MOKSHA_APKSIGNER}" sign --ks "${keystore}" --ks-pass pass:android --key-pass pass:android
                --out "${apk}" "${apkDir}/aligned.apk"
        COMMENT "Packaging ${target}.apk"
        VERBATIM)

    add_custom_target(${target}_install
        COMMAND "${MOKSHA_ADB}" install -r "${apk}"
        COMMAND "${MOKSHA_ADB}" shell am start -n "${appId}/android.app.NativeActivity"
        COMMENT "Installing and starting ${target}.apk"
        VERBATIM
        USES_TERMINAL)
    add_dependencies(${target}_install ${target})
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
    elseif(cookTarget STREQUAL "android-vulkan")
        # NativeActivity loads the game as a shared library.
        add_library(${target} SHARED ${ARG_SOURCES})
    elseif(cookTarget)
        add_executable(${target} WIN32 ${ARG_SOURCES})
    endif()
    if(cookTarget)
        target_include_directories(${target} PRIVATE "${PROJECT_SOURCE_DIR}")
        target_link_libraries(${target} PRIVATE Moksha_Entry Moksha_Client ${ARG_LIBRARIES})
        _moksha_add_cook(${target} ${cookTarget} "${MOKSHA_ENGINE_RES_LIST}" "${resList}")
        if(cookTarget STREQUAL "web-webgpu")
            _moksha_web_page(${target} "${ARG_DISPLAY_NAME}")
        elseif(cookTarget STREQUAL "android-vulkan")
            _moksha_app_id(appId ${target} "${ARG_BUNDLE_ID}" "_")
            _moksha_android_apk(${target} "${appId}" "${ARG_DISPLAY_NAME}" "${ARG_ORIENTATION}")
        endif()
    elseif(TARGET Moksha_Client)
        message(WARNING "moksha_add_game(${target}): no windowed game on '${CMAKE_SYSTEM_NAME}' "
                        "(${MOKSHA_RENDERER}) yet; only the headless one is created")
    endif()

    # Headless twin: same sources, no window, GPU or audio device; a console program on every desktop OS, and a Node
    # program on web, reading the real disk. iOS and Android run only apps.
    if(ARG_HEADLESS AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS" AND NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
        set(headless ${target}_Headless)
        add_executable(${headless} ${ARG_SOURCES})
        target_include_directories(${headless} PRIVATE "${PROJECT_SOURCE_DIR}")
        target_link_libraries(${headless} PRIVATE Moksha_EntryHeadless Moksha_ClientHeadless ${ARG_LIBRARIES})
        if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
            target_link_options(${headless} PRIVATE -sENVIRONMENT=node -sNODERAWFS=1 -sALLOW_MEMORY_GROWTH=1)
        endif()
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
    # Servers are console programs; iOS and Android run only apps, and a browser can't host a server.
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS" OR CMAKE_SYSTEM_NAME STREQUAL "Android" OR CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
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
