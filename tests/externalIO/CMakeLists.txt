include(ExternalProject)
project(netchasm)
add_custom_target(
	CertGenerator
	COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/certs/cert.sh ${CMAKE_CURRENT_SOURCE_DIR}/certs
     )

add_definitions(-DCERT_FOLDER="${CMAKE_CURRENT_BINARY_DIR}")
if(NOT SKIP-MDBM)
	list(APPEND SOURCES "TestHMCommandListenerUtils.cpp" "TestHMControlLinuxSocket.cpp" "TestHMControlLinuxSocket1.cpp" "TestHMControlLinuxSocket2.cpp"
		     "TestHMControlLinuxSocket3.cpp" "TestHMControlLinuxSocket4.cpp" "TestHMControlLinuxSocket5.cpp"
		     "TestHMControlTCPSocket.cpp" "TestHMControlTCPSocket2.cpp" "TestHMControlTCPSocket5.cpp"
		     "TestHMControlTLSSocket.cpp" "TestHMControlTLSSocket2.cpp" "TestHMControlTLSSocket5.cpp" "TestHMControlTLSSocket6.cpp"
		     "TestHMControlTLSSocket7.cpp" "TestHMControlTLSSocket9.cpp")
	if(NOT SKIP-IPV6)
		list(APPEND SOURCES "TestHMControlTCPSocket1.cpp" "TestHMControlTCPSocket3.cpp" "TestHMControlTCPSocket4.cpp"
				     "TestHMControlTLSSocket1.cpp" "TestHMControlTLSSocket3.cpp" "TestHMControlTLSSocket4.cpp")
	endif()

endif()
add_executable(externalio ${SOURCES} ../shared/common.cpp ../CppUnitTestRunner.cc)
set_property(TARGET externalio PROPERTY CXX_STANDARD 14)
add_dependencies(externalio CertGenerator)
add_test(externalio externalio)
