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
    server/address.cc
    server/bytearray.cc
    server/config.cc
    server/fd_manager.cc
    server/fiber.cc
    server/hook.cc 
    server/http/http.cc
    server/http/http_parser.cc
    server/http/http_server.cc
    server/http/http_session.cc
    server/http/http11_parser.rl.cc
    server/http/httpclient_parser.rl.cc
    server/iomanager.cc  
    server/log.cc 
    server/scheduler.cc
    server/socket.cc
    server/socket_stream.cc
    server/stream.cc
    server/tcp_server.cc
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

add_executable(test_address  tests/test_address.cc)
add_dependencies(test_address server)
force_redefine_file_macro_for_sources(test_address) #__FILE__
target_link_libraries(test_address server ${LIB_LIB})

add_executable(test_socket  tests/test_socket.cc)
add_dependencies(test_socket server)
force_redefine_file_macro_for_sources(test_socket) #__FILE__
target_link_libraries(test_socket server ${LIB_LIB})

add_executable(test_bytearray  tests/test_bytearray.cc)
add_dependencies(test_bytearray server)
force_redefine_file_macro_for_sources(test_bytearray) #__FILE__
target_link_libraries(test_bytearray server ${LIB_LIB})

add_executable(test_http  tests/test_http.cc)
add_dependencies(test_http server)
force_redefine_file_macro_for_sources(test_http) #__FILE__
target_link_libraries(test_http server ${LIB_LIB})

add_executable(test_httpparser  tests/test_httpparser.cc)
add_dependencies(test_httpparser server)
force_redefine_file_macro_for_sources(test_httpparser) #__FILE__
target_link_libraries(test_httpparser server ${LIB_LIB})

add_executable(test_tcp_server  tests/test_tcp_server.cc)
add_dependencies(test_tcp_server server)
force_redefine_file_macro_for_sources(test_tcp_server) #__FILE__
target_link_libraries(test_tcp_server server ${LIB_LIB})

add_executable(echo_server  examples/echo_server.cc)
add_dependencies(echo_server server)
force_redefine_file_macro_for_sources(echo_server) #__FILE__
target_link_libraries(echo_server server ${LIB_LIB})

add_executable(test_http_server  tests/test_http_server.cc)
add_dependencies(test_http_server server)
force_redefine_file_macro_for_sources(test_http_server) #__FILE__
target_link_libraries(test_http_server server ${LIB_LIB})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)