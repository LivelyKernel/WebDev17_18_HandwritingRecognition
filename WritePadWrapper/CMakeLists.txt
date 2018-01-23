cmake_minimum_required(VERSION 3.0)
project(WritePadWrapper)

set(CMAKE_CXX_STANDARD 11)
add_compile_options(-std=c++11)

include_directories(
        lib
        include
)

set(HEADER_FILES
        include/gestures.h
        include/InkWrapper.h
        include/langid.h
        include/recodefs.h
        include/RecognizerApi.h
        include/RecognizerWrapper.h
        include/recotypes.h
        include/json/json.h
        include/json/json-forwards.h
        include/jsoncpp.cpp)
set(SOURCE_FILES main.cpp)

add_library(WritePadWrapper SHARED ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(WritePadWrapper ${CMAKE_SOURCE_DIR}/lib/libWritePadLib.a)

IF(APPLE)
SET(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
ENDIF(APPLE)
