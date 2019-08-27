include(ExternalProject)
project(netchasm)
add_custom_target(
	CertGenerator
	COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/certs/cert.sh ${CMAKE_CURRENT_SOURCE_DIR}/certs
     )

add_definitions(-DCERT_FOLDER="${CMAKE_CURRENT_BINARY_DIR}")
file(GLOB SOURCES "./*.cpp")
add_executable(externalio ${SOURCES} ../shared/common.cpp ../CppUnitTestRunner.cc)
set_property(TARGET externalio PROPERTY CXX_STANDARD 14)
add_dependencies(externalio CertGenerator)
add_test(externalio externalio)
