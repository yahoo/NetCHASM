cmake_minimum_required(VERSION 2.8)

project(netchasm)

set(CPP_FLAGS "-Wall -Wextra -Wchkp -Wfloat-equal")

message( hm_dumpconfig )
add_executable( hm_dumpconfig hm_dumpconfig.cpp)
set_property(TARGET hm_dumpconfig PROPERTY CXX_STANDARD 14)
set_target_properties(hm_dumpconfig PROPERTIES COMPILE_FLAGS "${CPP_FLAGS}")
set_target_properties(hm_dumpconfig PROPERTIES LINK_FLAGS "${CPP_FLAGS}")
target_link_libraries (hm_dumpconfig netchasm_core)
target_link_libraries (hm_dumpconfig netchasm)
install(TARGETS hm_dumpconfig RUNTIME DESTINATION bin64)
