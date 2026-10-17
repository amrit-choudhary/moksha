# Guard rail for live-linked games: a game normally builds the engine commit it pins (MOKSHA_PIN, its GIT_TAG),
# but with FETCHCONTENT_SOURCE_DIR_MOKSHA it builds this local clone instead. Warns when the pin differs from the
# clone's HEAD, so the pin gets bumped (after pushing the engine) before the game is pushed.

if(NOT FETCHCONTENT_SOURCE_DIR_MOKSHA OR NOT MOKSHA_PIN)
    return()
endif()

find_package(Git QUIET)
if(NOT GIT_FOUND)
    return()
endif()

execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    OUTPUT_VARIABLE mokshaHead OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE mokshaResult ERROR_QUIET)
if(NOT mokshaResult EQUAL 0)
    return()
endif()

execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse --verify --quiet "${MOKSHA_PIN}^{commit}"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    OUTPUT_VARIABLE mokshaPin OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE mokshaResult ERROR_QUIET)
if(NOT mokshaResult EQUAL 0)
    message(WARNING "Moksha: the game's engine pin ${MOKSHA_PIN} is not in the local engine clone "
                    "(${PROJECT_SOURCE_DIR}). Pull the engine, or check the pin.")
    return()
endif()

if(mokshaPin STREQUAL mokshaHead)
    return()
endif()

execute_process(COMMAND "${GIT_EXECUTABLE}" rev-list --count "${mokshaPin}..${mokshaHead}"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    OUTPUT_VARIABLE mokshaBehind OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
execute_process(COMMAND "${GIT_EXECUTABLE}" rev-list --count "${mokshaHead}..${mokshaPin}"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    OUTPUT_VARIABLE mokshaAhead OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

string(SUBSTRING "${mokshaPin}" 0 10 mokshaPinShort)
string(SUBSTRING "${mokshaHead}" 0 10 mokshaHeadShort)
if(mokshaAhead STREQUAL "0")
    message(WARNING "Moksha: the game pins engine ${mokshaPinShort}, ${mokshaBehind} commit(s) behind the local "
                    "engine HEAD ${mokshaHeadShort} that this live build uses. Push the engine, then bump MOKSHA_PIN.")
else()
    message(WARNING "Moksha: the game pins engine ${mokshaPinShort}, which the local engine HEAD "
                    "${mokshaHeadShort} doesn't contain (${mokshaAhead} commit(s) missing). Pull or switch the "
                    "engine clone.")
endif()
