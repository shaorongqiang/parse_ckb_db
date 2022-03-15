cmake_minimum_required(VERSION 3.18)

file(RELATIVE_PATH project_name ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR})

set(_VERSION "0.0.1")
ADD_DEFINITIONS(-D_VERSION=\"${_VERSION}\")
project (${project_name}  VERSION ${_VERSION})

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")  #设置c++的编译选项
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")  #设置c的编译选项

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(EXECUTABLE_OUTPUT_PATH bin)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE debug)
endif()

execute_process(COMMAND sh ${PROJECT_SOURCE_DIR}/make_depend.sh ${PROJECT_SOURCE_DIR}/3rd ${CMAKE_CURRENT_BINARY_DIR}/3rd)

include_directories( ${PROJECT_SOURCE_DIR})
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/3rd/protobuf/src )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/3rd/nlohmann_json/include )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/3rd/libevent/include )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/3rd/libevent/build/include )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/3rd/rocksdb/include )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/3rd/libbase58 )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/3rd/spdlog/include )

add_library(protobuf STATIC IMPORTED)
set_property(TARGET protobuf PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/3rd/protobuf/build/libprotobuf.a)

add_library(cryptopp STATIC IMPORTED)
set_property(TARGET cryptopp PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/3rd/cryptopp/libcryptopp.a)

add_library(event STATIC IMPORTED)
set_property(TARGET event PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/3rd/libevent/build/lib/libevent.a)

add_library(base58 STATIC IMPORTED)
set_property(TARGET base58 PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/3rd/libbase58/.libs/libbase58.a)

add_library(rocksdb STATIC IMPORTED)
set_property(TARGET rocksdb PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/3rd/rocksdb/build/librocksdb.a)

add_library(spdlog STATIC IMPORTED)
set_property(TARGET spdlog PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/3rd/spdlog/build/libspdlog.a)

# 编译那些源码
file(GLOB SOURCES_FILES
    "db/*.cpp"
    "log/*.cpp"
    "utils/*.cpp"
    "molecule/*.cpp"
    )
#message(${SOURCES_FILES})

add_executable(${PROJECT_NAME} main.cpp ${SOURCES_FILES})

target_link_libraries(${PROJECT_NAME} protobuf )
target_link_libraries(${PROJECT_NAME} cryptopp )
target_link_libraries(${PROJECT_NAME} event )
target_link_libraries(${PROJECT_NAME} base58 )
target_link_libraries(${PROJECT_NAME} rocksdb )
target_link_libraries(${PROJECT_NAME} spdlog )
target_link_libraries(${PROJECT_NAME}  -lpthread -lsnappy -lstdc++fs -static-libgcc -static-libstdc++ -ldl)

find_package(GTest)
if(GTEST_FOUND)
    include_directories(${GTEST_INCLUDE_DIRS})
    file(GLOB_RECURSE TEST_SOURCE test/*.cpp)
    add_executable(gtest EXCLUDE_FROM_ALL ${TEST_SOURCE})
    target_link_libraries(gtest ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(gtest protobuf )
    target_link_libraries(gtest cryptopp )
    target_link_libraries(gtest event )
    target_link_libraries(gtest base58 )
    target_link_libraries(gtest rocksdb )
    target_link_libraries(gtest spdlog )
    target_link_libraries(gtest  -lpthread -lsnappy -lstdc++fs -static-libgcc -static-libstdc++ -ldl)
endif(GTEST_FOUND)
