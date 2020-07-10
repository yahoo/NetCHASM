include_directories(shared)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -Wchkp -Wfloat-equal -fprofile-arcs -ftest-coverage")

if(ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -static-libasan")
endif(ASAN)

if(NOT SKIP-ARES)
  ADD_DEFINITIONS(-DUSE_ARES)
endif()

execute_process (
    COMMAND export CPPUNITTESTENVARGS = LSAN_OPTIONS=suppressions=$(LSANSUPPRESSIONSFILE) ASAN_OPTIONS=fast_unwind_on_malloc=0:detect_stack_use_after_return=true:check_initialization_order=true
)
link_libraries (pthread)
link_libraries (cppunit)
link_libraries (netchasm)

add_subdirectory(configParserTests)
add_subdirectory(coreTests)
add_subdirectory(externalIO)
add_subdirectory(logTests)
add_subdirectory(storeTests)
add_subdirectory(workTests)

