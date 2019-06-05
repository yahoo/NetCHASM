cmake_minimum_required(VERSION 2.8)

project(netchasm)

set(CPP_FLAGS "-Wall -Wextra -Wchkp -Wfloat-equal")

message( hm_set )
add_executable( hm_set hm_set.cpp)
set_property(TARGET hm_set PROPERTY CXX_STANDARD 14)
set_target_properties(hm_set PROPERTIES COMPILE_FLAGS "${CPP_FLAGS}")
set_target_properties(hm_set PROPERTIES LINK_FLAGS "${CPP_FLAGS}")
target_link_libraries (hm_set netchasm_core)
install(TARGETS hm_set RUNTIME DESTINATION bin64)
