include(ExternalProject)
project(netchasm)

list(APPEND SOURCES "TestHMDataCheckList.cpp" "TestHMDataCheckParams.cpp" "TestHMDataHostCheck.cpp" "TestHMDNSCache.cpp"
		    "TestHMDNSResult.cpp" "TestHMEventQueue.cpp" "TestHMHash.cpp" "TestHMIPAddress.cpp" "TestHMPubSubDataPacking.cpp"
		    "TestHMThreadPool.cpp" "TestHMTimeStamp.cpp" "TestHMWorkQueue.cpp" "TestHMRemoteCache.cpp" "TestHMRemoteResult.cpp"
		    "TestHMRemoteHostCache.cpp" "TestHMState.cpp")

if(NOT SKIP-MDBM)
        list(APPEND SOURCES "TestHMStateManager.cpp")
endif()

if(NOT SKIP-RAPIDXML)
	list(APPEND SOURCES "TestHMAuxInfo.cpp")
endif()


if(NOT SKIP-ARES)
       list(APPEND SOURCES "TestHMDataCheckListAres.cpp")
endif()

add_executable(coretests ${SOURCES} ../shared/common.cpp ../shared/TestStorageHostGroup.cpp ../CppUnitTestRunner.cc)
set_property(TARGET coretests PROPERTY CXX_STANDARD 14)

add_test(coretests coretests)

