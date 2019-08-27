include(ExternalProject)
project(netchasm)


file(GLOB SOURCES "./*.cpp")
add_executable(configparsertests ${SOURCES} ../shared/common.cpp ../CppUnitTestRunner.cc)
set_property(TARGET configparsertests PROPERTY CXX_STANDARD 14)

add_test(configparser configparsertests)
