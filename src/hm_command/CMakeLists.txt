project(netchasm)

set(CPP_FLAGS "-Wall -Wextra -Wchkp -Wfloat-equal")

message( hm_command )
add_executable( hm_command hm_command.cpp)
set_property(TARGET hm_command PROPERTY CXX_STANDARD 14)
set_target_properties(hm_command PROPERTIES COMPILE_FLAGS "${CPP_FLAGS}")
set_target_properties(hm_command PROPERTIES LINK_FLAGS "${CPP_FLAGS}")
target_link_libraries (hm_command netchasm)
install(TARGETS hm_command RUNTIME DESTINATION bin64)
