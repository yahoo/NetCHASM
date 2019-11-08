cmake_minimum_required(VERSION 2.8)

project(netchasm) 

file(GLOB SOURCES "./*.cpp")
set(CMAKE_BUILD_TYPE Debug)
set(CPP_FLAGS "-Wall -Wextra -Wchkp -Wfloat-equal -Wl,--copy-dt-needed-entries -Wl,--as-needed -Wl,--no-undefined")
set(COV_FLAGS "-fprofile-arcs -ftest-coverage")
set(ASAN_FLAGS "-fsanitize=address -fno-omit-frame-pointer -static-libasan")

add_library(netchasm SHARED ${SOURCES})

set_target_properties(netchasm PROPERTIES SOVERSION "${MAJOR_VERSION}")
set_target_properties(netchasm PROPERTIES COMPILE_FLAGS "${CPP_FLAGS} ")
set_target_properties(netchasm PROPERTIES LINK_FLAGS "${CPP_FLAGS}")

if(COV)
    set_target_properties(netchasm PROPERTIES COMPILE_FLAGS "${COV_FLAGS}")
    set_target_properties(netchasm PROPERTIES LINK_FLAGS "${COV_FLAGS}")
endif(COV)

set_property(TARGET netchasm PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET netchasm PROPERTY CXX_STANDARD 14)

target_link_libraries (netchasm netchasm_core)

install(TARGETS netchasm LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
