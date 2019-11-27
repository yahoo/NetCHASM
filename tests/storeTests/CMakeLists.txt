include(ExternalProject)
project(netchasm)

list(APPEND SOURCES "TestHMStorage.cpp" "TestHMStorageHost.cpp" "TestHMStorageHostGroup.cpp" "TestHMStorageText.cpp")
if(NOT SKIP-MDBM)
	list(APPEND SOURCES "TestHMStorageHostGroupMDBM.cpp")
endif()

add_executable(storetests ${SOURCES} ../shared/common.cpp ../shared/TestStorage.cpp ../shared/TestStorageHost.cpp ../shared/TestStorageHostGroup.cpp ../CppUnitTestRunner.cc)
set_property(TARGET storetests PROPERTY CXX_STANDARD 14)

add_test(storetests storetests)
