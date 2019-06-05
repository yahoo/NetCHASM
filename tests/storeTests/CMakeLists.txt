cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
project(bkln_netchasm)

file(GLOB SOURCES "./*.cpp")
add_executable(storetests ${SOURCES} ../shared/common.cpp ../shared/TestStorage.cpp ../shared/TestStorageHost.cpp ../shared/TestStorageHostGroup.cpp ../CppUnitTestRunner.cc)
set_property(TARGET storetests PROPERTY CXX_STANDARD 14)

add_test(storetests storetests)