project(netchasm)
message( include/NetCHASM )
install(DIRECTORY "." DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/NetCHASM
              FILES_MATCHING PATTERN "*.h")

