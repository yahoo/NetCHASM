cmake_minimum_required(VERSION 2.8)

project(netchasm)

set(CPP_FLAGS "-Wall -Wextra -Wchkp -Wfloat-equal")

message( NetCHASMDaemon )
add_executable( NetCHASMDaemon NetCHASMDaemon.cpp)
set_property(TARGET NetCHASMDaemon PROPERTY CXX_STANDARD 14)
set_target_properties(NetCHASMDaemon PROPERTIES COMPILE_FLAGS "${CPP_FLAGS}")
set_target_properties(NetCHASMDaemon PROPERTIES LINK_FLAGS "${CPP_FLAGS}")
target_link_libraries (NetCHASMDaemon netchasm_core)
install(TARGETS NetCHASMDaemon RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
