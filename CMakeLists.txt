cmake_minimum_required(VERSION 3.11)
project("NetCHASM")
include(ExternalProject)
include(GNUInstallDirs)
enable_testing()
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--enable-new-dtags")
set(OBJECT_DIR ${CMAKE_BINARY_DIR})
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "..." FORCE)
endif()
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(MAJOR_VERSION 1)
set(MINOR_VERSION 3)
set(PATCH_VERSION 0)

option(ASAN "enable ASAN options" OFF)
option(COV "enable CODE COVERAGE options" OFF)
option(SKIP-LIBEVENT "skip LIBEVENT library and related code" OFF)
option(SKIP-ARES "skip ARES library and related code" OFF)
option(SKIP-MDBM "skip MDBM library and related code" OFF)
option(SKIP-KAFKA "skip KAFKA library and related code" OFF)
option(SKIP-IPV6 "skip IPv6 tests" OFF)

link_directories(/lib64)
if(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
    INCLUDE(InstallRequiredSystemLibraries)
    set(CPACK_RPM_COMPONENT_INSTALL ON)
    set(CMAKE_INSTALL_TMPDIR /tmp CACHE PATH "Output dir for tmp")
    set(CPACK_PACKAGE_NAME "NetCHASM")
    set(CPACK_RPM_DIST_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
    set(CPACK_RPM_DEVEL_PACKAGE_NAME ${CPACK_PACKAGE_NAME}-devel)
    set(CPACK_PACKAGE_VERSION_MAJOR ${MAJOR_VERSION})
    set(CPACK_PACKAGE_VERSION_MINOR ${MINOR_VERSION})
    set(CPACK_PACKAGE_VERSION_PATCH ${PATCH_VERSION})
    set(CPACK_GENERATOR "RPM")
    set(CPACK_SOURCE_IGNORE_FILES "rdkafka")
    set(CPACK_COMPONENTS_ALL DIST DEVEL)
    set(CPACK_PACKAGE_RELOCATABLE NO)
    set(CPACK_COMPONENTS_GROUPING ONE_PER_GROUP)
    set(CPACK_RPM_PACKAGE_REQUIRES "curl >= 7.61.0, yaml-cpp >= 0.6, librdkafka >= 1.2, protobuf >= 3.6, c-ares")
    set(CPACK_RPM_PACKAGE_REQUIRES_PRE "curl, yaml-cpp, librdkafka, protobuf, c-ares")
    set(CPACK_RPM_PACKAGE_REQUIRES_POSTUN "curl, yaml-cpp, librdkafka, protobuf, c-ares")
    set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
    set(CPACK_RPM_PACKAGE_AUTOREQPROV NO)
    set(CPACK_RPM_PACKAGE_URL "https://github.com/yahoo/NetCHASM")
    set(CPACK_RPM_DIST_FILE_NAME "${CPACK_RPM_DIST_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}${RELEASE-STR}.rpm")
    set(CPACK_RPM_DEVEL_FILE_NAME "${CPACK_RPM_DEVEL_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}${RELEASE-STR}.rpm")
    include(CPack)
endif()

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
