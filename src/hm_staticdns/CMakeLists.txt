project(netchasm)

set(CPP_FLAGS "-Wall -Wextra -Wchkp -Wfloat-equal")

message( hm_staticdns )
add_executable( hm_staticdns hm_staticdns.cpp)
set_property(TARGET hm_staticdns PROPERTY CXX_STANDARD 14)
set_target_properties(hm_staticdns PROPERTIES COMPILE_FLAGS "${CPP_FLAGS}")
set_target_properties(hm_staticdns PROPERTIES LINK_FLAGS "${CPP_FLAGS}")
target_link_libraries (hm_staticdns netchasm)
install(TARGETS hm_staticdns RUNTIME COMPONENT DIST DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES hm_staticdns.1 COMPONENT DIST DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
