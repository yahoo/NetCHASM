project(netchasm)

set(CPP_FLAGS "-Wall -Wextra -Wchkp -Wfloat-equal")

message( hm_reload )
add_executable( hm_reload hm_reload.cpp)
set_property(TARGET hm_reload PROPERTY CXX_STANDARD 14)
set_target_properties(hm_reload PROPERTIES COMPILE_FLAGS "${CPP_FLAGS}")
set_target_properties(hm_reload PROPERTIES LINK_FLAGS "${CPP_FLAGS}")
target_link_libraries (hm_reload netchasm)
install(TARGETS hm_reload RUNTIME DESTINATION bin64)
