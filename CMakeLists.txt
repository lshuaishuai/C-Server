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
    server/config.cc
    server/fd_manager.cc
    server/fiber.cc
    server/hook.cc 
    server/iomanager.cc  
    server/log.cc 
    server/scheduler.cc
    server/thread.cc
    server/timer.cc  
    server/util.cc  
    )

add_library(server SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(server) #__FILE__
# add_library(shuai_static STATIC ${LIB_SRC})
# SET_TARGET_PROPERTIES (shuai_static PROPERTIES OUTPUT_NAME "shuai")

set(LIB_LIB
    dl
    server
    pthread
    ${YAMLCPP}
    )

add_executable(test tests/test.cc)
add_dependencies(test server)
force_redefine_file_macro_for_sources(test) #__FILE__
target_link_libraries(test server ${LIB_LIB})

add_executable(test_config  tests/test_config.cc)
add_dependencies(test_config server)
force_redefine_file_macro_for_sources(test_config) #__FILE__
target_link_libraries(test_config server ${LIB_LIB})

add_executable(test_thread  tests/test_thread.cc)
add_dependencies(test_thread server)
force_redefine_file_macro_for_sources(test_thread) #__FILE__
target_link_libraries(test_thread server ${LIB_LIB})

add_executable(test_util  tests/test_util.cc)
add_dependencies(test_util server)
force_redefine_file_macro_for_sources(test_util) #__FILE__
target_link_libraries(test_util server ${LIB_LIB})

add_executable(test_fiber  tests/test_fiber.cc)
add_dependencies(test_fiber server)
force_redefine_file_macro_for_sources(test_fiber) #__FILE__
target_link_libraries(test_fiber server ${LIB_LIB})

add_executable(test_scheduler  tests/test_scheduler.cc)
add_dependencies(test_scheduler server)
force_redefine_file_macro_for_sources(test_scheduler) #__FILE__
target_link_libraries(test_scheduler server ${LIB_LIB})

add_executable(test_iomanager  tests/test_iomanager.cc)
add_dependencies(test_iomanager server)
force_redefine_file_macro_for_sources(test_iomanager) #__FILE__
target_link_libraries(test_iomanager server ${LIB_LIB})

add_executable(test_hook  tests/test_hook.cc)
add_dependencies(test_hook server)
force_redefine_file_macro_for_sources(test_hook) #__FILE__
target_link_libraries(test_hook server ${LIB_LIB})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)