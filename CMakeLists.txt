cmake_minimum_required(VERSION 3.11)

include(ExternalProject)
enable_testing()
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--enable-new-dtags")
set(OBJECT_DIR ${CMAKE_BINARY_DIR})
set(CMAKE_INSTALL_PREFIX "/bin")
set(MAJOR_VERSION 1)

option(ASAN "enable ASAN options" OFF)
option(COV "enable CODE COVERAGE options" OFF)
option(SKIP-LIBEVENT "skip LIBEVENT library and related code" OFF)
option(SKIP-ARES "skip ARES library and related code" OFF)
option(SKIP-MDBM "skip MDBM library and related code" OFF)
option(SKIP-KAFKA "skip KAFKA library and related code" OFF)
option(SKIP-IPV6 "skip IPv6 tests" OFF)

link_directories(/lib64)

add_custom_target(build
    COMMAND ${CMAKE_MAKE_PROGRAM}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

add_custom_target(testbuild
    COMMAND ${CMAKE_MAKE_PROGRAM}
    COMMAND ${CMAKE_MAKE_PROGRAM} configparsertests
    COMMAND ${CMAKE_MAKE_PROGRAM} coretests
    COMMAND ${CMAKE_MAKE_PROGRAM} externalio
    COMMAND ${CMAKE_MAKE_PROGRAM} logtests
    COMMAND ${CMAKE_MAKE_PROGRAM} storetests
    COMMAND ${CMAKE_MAKE_PROGRAM} worktests
    COMMAND ${CMAKE_MAKE_PROGRAM} test ARGS="-V"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

add_custom_command(TARGET testbuild
    COMMAND lcov -c --directory ${OBJECT_DIR} --gcov-tool gcov --output-file netchasm.info
    COMMAND lcov --remove netchasm.info 'usr/include/cppunit/*' 'tests/*' -o netchasm.info
    COMMAND genhtml netchasm.info -o report
    )

add_subdirectory(proto)
add_subdirectory(src/internal)
add_subdirectory(src/api)
add_subdirectory(src/NetCHASMDaemon)
add_subdirectory(src/hm_command)
add_subdirectory(src/hm_reload)
add_subdirectory(src/hm_remote)
add_subdirectory(src/hm_staticdns)
add_subdirectory(tests EXCLUDE_FROM_ALL)
