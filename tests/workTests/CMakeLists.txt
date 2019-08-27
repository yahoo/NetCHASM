include(ExternalProject)
project(netchasm)

file(GLOB SOURCES "./*.cpp")
add_executable(worktests ${SOURCES} ../shared/common.cpp ../shared/TestStorage.cpp ../CppUnitTestRunner.cc)
set_property(TARGET worktests PROPERTY CXX_STANDARD 14)

add_test(worktests worktests)
