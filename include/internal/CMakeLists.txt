project(netchasm)
message( include/internal )
install(DIRECTORY "." DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/NetCHASM/internal
              FILES_MATCHING PATTERN "*.h")

