cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
project(bkln_netchasm)

file(GLOB SOURCES "./*.cpp")
add_executable(externalio ${SOURCES} ../shared/common.cpp ../CppUnitTestRunner.cc)
set_property(TARGET externalio PROPERTY CXX_STANDARD 14)

add_test(externalio externalio)