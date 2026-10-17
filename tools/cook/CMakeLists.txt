# Moksha_Cook: host tool that cooks res_list.json resources for one target platform (see cook.h).
# moksha_add_game runs it before each game build; cross-compiled builds use MOKSHA_COOK_EXECUTABLE instead.

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(Moksha_Cook ${SOURCES})

target_link_libraries(Moksha_Cook PRIVATE Moksha_Shared)
