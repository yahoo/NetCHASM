include(ExternalProject)
project(netchasm) 

file(GLOB SOURCES "./*.cpp")

if(NOT SKIP-MDBM)
  list(APPEND SOURCES ./storage_mdbm/HMStorageHostGroupMDBM.cpp)
endif()

if(NOT SKIP-ARES)
  list(APPEND SOURCES ./checks_ares/HMWorkDNSLookupAres.cpp)
  list(APPEND SOURCES ./checks_ares/HMWorkHealthCheckDNS.cpp)
endif()

if(NOT SKIP-LIBEVENT)
  list(APPEND SOURCES ./checks_libevent/HMEventLoopLibEvent.cpp)
  list(APPEND SOURCES ./checks_libevent/HMWorkDNSLookupLibEvent.cpp)
  list(APPEND SOURCES ./checks_libevent/HMWorkHealthCheckLibEvent.cpp)
endif()

if(NOT SKIP-RAPIDXML)
  list(APPEND SOURCES ./rapidxml_parser/HMAuxParserRapidXML.cpp)
endif()

if(NOT SKIP-KAFKA)
  list(APPEND SOURCES ./publisher_kafka/HMPublisherKafka.cpp)
endif()

set(CMAKE_BUILD_TYPE Debug)
set(CPP_FLAGS "-Wall -Wextra -Wchkp -Wfloat-equal -Wl,--copy-dt-needed-entries -Wl,--as-needed -Wl,--no-undefined")
set(COV_FLAGS "-fprofile-arcs -ftest-coverage")
set(ASAN_FLAGS "-fsanitize=address -fno-omit-frame-pointer -static-libasan")

add_library(netchasm_core STATIC ${SOURCES})

set_target_properties(netchasm_core PROPERTIES COMPILE_FLAGS "${CPP_FLAGS} ")
set_target_properties(netchasm_core PROPERTIES LINK_FLAGS "${CPP_FLAGS}")

if(COV)
    set_target_properties(netchasm_core PROPERTIES COMPILE_FLAGS "${COV_FLAGS}")
    set_target_properties(netchasm_core PROPERTIES LINK_FLAGS "${COV_FLAGS}")
endif(COV)

target_include_directories(netchasm_core PUBLIC ${CMAKE_SOURCE_DIR}/include/internal ${CMAKE_SOURCE_DIR}/include/NetCHASM)

target_link_libraries (netchasm_core PUBLIC netchasm_proto)  
target_link_libraries (netchasm_core PUBLIC curl ssl crypto pthread gcov protobuf)
target_link_libraries (netchasm_core PRIVATE yaml-cpp)

if(NOT SKIP-KAFKA)
  ADD_DEFINITIONS(-DUSE_KAFKA)
  target_link_libraries (netchasm_core PUBLIC rdkafka rdkafka++)
endif()

if(NOT SKIP-ARES)
  ADD_DEFINITIONS(-DUSE_ARES)
  target_link_libraries (netchasm_core PRIVATE cares)
endif()

if(NOT SKIP-MDBM)
  ADD_DEFINITIONS(-DUSE_MDBM)
  target_link_libraries (netchasm_core PUBLIC mdbm)
endif()

if(NOT SKIP-LIBEVENT)
  ADD_DEFINITIONS(-DUSE_LIBEVENT)
  target_link_libraries (netchasm_core PUBLIC event event_openssl)
endif()

if(NOT SKIP-RAPIDXML)
  ADD_DEFINITIONS(-DUSE_RAPIDXML)
endif()


set_property(TARGET netchasm_core PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET netchasm_core PROPERTY CXX_STANDARD 14)

