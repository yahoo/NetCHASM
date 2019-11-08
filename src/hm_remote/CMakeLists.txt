project(netchasm)

set(CPP_FLAGS "-Wall -Wextra -Wchkp -Wfloat-equal")

message( hm_remote )
add_executable( hm_remote hm_remote.cpp)
set_property(TARGET hm_remote PROPERTY CXX_STANDARD 14)
set_target_properties(hm_remote PROPERTIES COMPILE_FLAGS "${CPP_FLAGS}")
set_target_properties(hm_remote PROPERTIES LINK_FLAGS "${CPP_FLAGS}")
target_link_libraries (hm_remote netchasm)
install(TARGETS hm_remote RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
