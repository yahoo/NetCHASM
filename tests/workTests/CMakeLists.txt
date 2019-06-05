cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
project(bkln_netchasm)

file(GLOB SOURCES "./*.cpp")
add_executable(worktests ${SOURCES} ../shared/common.cpp ../shared/TestStorage.cpp ../CppUnitTestRunner.cc)
set_property(TARGET worktests PROPERTY CXX_STANDARD 14)

add_test(worktests worktests)