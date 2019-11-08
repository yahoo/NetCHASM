project(netchasm_proto CXX) 

list(APPEND PROTO netchasm/ipaddress.proto)
list(APPEND PROTO netchasm/auxinfo.proto)
list(APPEND PROTO netchasm/datacheckresult.proto)
list(APPEND PROTO netchasm/datahostgroup.proto)
list(APPEND PROTO netchasm/datahostcheck.proto)
list(APPEND PROTO netchasm/generalparams.proto)
list(APPEND PROTO netchasm/hostgroupinfo.proto)
list(APPEND PROTO netchasm/hostresults.proto)
list(APPEND PROTO netchasm/hostschdinfo.proto)
list(APPEND PROTO netchasm/threadinfo.proto)

foreach( file  ${PROTO})
    message( "Generating proto buf files for ${file}" )
    string( REPLACE ".proto" ".pb.cc" name ${file})
    set (SOURCES ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/${name})
    add_custom_command(
	OUTPUT  ${CMAKE_CURRENT_SOURCE_DIR}/${name}
        COMMAND ${PROTOBUF_PATH}/bin/protoc --proto_path=${CMAKE_CURRENT_SOURCE_DIR} 
        --cpp_out=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${file}
        COMMAND ${PROTOBUF_PATH}/bin/protoc --proto_path=${CMAKE_CURRENT_SOURCE_DIR} 
        --python_out=${CMAKE_SOURCE_DIR}/api ${CMAKE_CURRENT_SOURCE_DIR}/${file}
	DEPENDS ${file}
     )

endforeach( file ${PROTO} )

set(CMAKE_BUILD_TYPE Debug)
set(CPP_FLAGS "-Wall -Wextra -Wchkp -Wfloat-equal -Wl,--copy-dt-needed-entries -Wl,--as-needed -Wl,--no-undefined")
set(COV_FLAGS "-fprofile-arcs -ftest-coverage")
set(ASAN_FLAGS "-fsanitize=address -fno-omit-frame-pointer -static-libasan")
add_library(netchasm_proto STATIC ${SOURCES})
target_include_directories(netchasm_proto PUBLIC ${CMAKE_SOURCE_DIR}/proto PUBLIC ${PROTOBUF_PATH}/include)

set_target_properties(netchasm_proto PROPERTIES COMPILE_FLAGS "${CPP_FLAGS} ")
set_target_properties(netchasm_proto PROPERTIES LINK_FLAGS "${CPP_FLAGS}")

if(COV)
    set_target_properties(netchasm_proto PROPERTIES COMPILE_FLAGS "${COV_FLAGS}")
    set_target_properties(netchasm_proto PROPERTIES LINK_FLAGS "${COV_FLAGS}")
endif(COV)

set_property(TARGET netchasm_proto PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET netchasm_proto PROPERTY CXX_STANDARD 14)

install(DIRECTORY "." DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/netchasm
   FILES_MATCHING PATTERN "*.h")
