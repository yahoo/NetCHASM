include(ExternalProject)
project(netchasm)

file(GLOB SOURCES "./*.cpp")
add_executable(coretests ${SOURCES} ../shared/common.cpp ../CppUnitTestRunner.cc)
set_property(TARGET coretests PROPERTY CXX_STANDARD 14)

add_test(coretests coretests)

