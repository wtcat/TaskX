zephyr_library_named(lua)

zephyr_library_include_directories(src)
zephyr_library_sources(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lbaselib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lmathlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lstring.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lparser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ldo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lutf8lib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lzio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lctype.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lstrlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lundump.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ldebug.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lobject.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ldump.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ldblib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ltm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ltable.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lstate.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/lua.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/loadlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lcorolib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lcode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lapi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lgc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lvm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lfunc.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/luac.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lauxlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ltablib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/linit.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lopcodes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/llex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/liolib.c

    ${CMAKE_CURRENT_SOURCE_DIR}/lua_api.c
)

zephyr_library_sources_ifdef(CONFIG_LUA_OS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/loslib.c
)
