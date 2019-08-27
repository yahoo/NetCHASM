project(netchasm)

set(CPP_FLAGS "-Wall -Wextra -Wchkp -Wfloat-equal")

message( hm_configure )
add_executable( hm_configure hm_configure.cpp)
set_property(TARGET hm_configure PROPERTY CXX_STANDARD 14)
set_target_properties(hm_configure PROPERTIES COMPILE_FLAGS "${CPP_FLAGS}")
set_target_properties(hm_configure PROPERTIES LINK_FLAGS "${CPP_FLAGS}")
target_link_libraries (hm_configure netchasm)
install(TARGETS hm_configure RUNTIME DESTINATION bin64)
