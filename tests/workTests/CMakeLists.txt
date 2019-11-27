include(ExternalProject)
project(netchasm)

list(APPEND SOURCES "TestHMWork.cpp" "TestHMWorkAuxFetch.cpp" "TestHMWorkDNSLookup.cpp" "TestHMWorkHealthCheck.cpp")
if(NOT SKIP-ARES)
	list(APPEND SOURCES "TestHMWorkAresTest.cpp")
endif()

add_executable(worktests ${SOURCES} ../shared/common.cpp ../shared/TestStorage.cpp ../CppUnitTestRunner.cc)
set_property(TARGET worktests PROPERTY CXX_STANDARD 14)

add_test(worktests worktests)
