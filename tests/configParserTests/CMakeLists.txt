include(ExternalProject)
project(netchasm)

list(APPEND SOURCES "TestHMConfigParserBase.cpp" "TestHMConfigParserYaml.cpp")


if(NOT SKIP-KAFKA)
	list(APPEND SOURCES "TestHMPubSubConfigParserYaml.cpp")
endif()

add_executable(configparsertests ${SOURCES} ../shared/common.cpp ../CppUnitTestRunner.cc)
set_property(TARGET configparsertests PROPERTY CXX_STANDARD 14)

add_test(configparser configparsertests)
