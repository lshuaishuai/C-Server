cmake_minimum_required(VERSION 3.0)
project(server)

include ( cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

include_directories(.)
include_directories(/usr/local/include/yaml-cpp/)

link_directories(/home/shuaishuai/project/sylar_server/include/yaml-cpp/build)

find_library(YAMLCPP yaml-cpp)
message("***", ${YAMLCPP})

set(LIB_SRC
    server/log.cc 
    server/util.cc  
    server/config.cc
    server/thread.cc
    )

add_library(server SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(server) #__FILE__
# add_library(shuai_static STATIC ${LIB_SRC})
# SET_TARGET_PROPERTIES (shuai_static PROPERTIES OUTPUT_NAME "shuai")

set(LIB_LIB
    server
    pthread
    ${YAMLCPP}
    )

# add_executable(test tests/test.cc)
# add_dependencies(test server)
# force_redefine_file_macro_for_sources(test) #__FILE__
# target_link_libraries(test server ${LIB_LIB})

add_executable(test_config  tests/test_config.cc)
add_dependencies(test_config server)
force_redefine_file_macro_for_sources(test_config) #__FILE__
target_link_libraries(test_config server ${LIB_LIB})

add_executable(test_thread  tests/test_thread.cc)
add_dependencies(test_thread server)
force_redefine_file_macro_for_sources(test_thread) #__FILE__
target_link_libraries(test_thread server ${LIB_LIB})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)