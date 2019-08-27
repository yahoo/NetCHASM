include(ExternalProject)
project(netchasm)

file(GLOB SOURCES "./*.cpp")
add_executable(logtests ${SOURCES} ../shared/common.cpp ../CppUnitTestRunner.cc)
set_property(TARGET logtests PROPERTY CXX_STANDARD 14)

add_test(logtests logtests)
