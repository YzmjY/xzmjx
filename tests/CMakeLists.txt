include_directories(../include)
link_directories(../lib)
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -g -std=c++14  -Wall -Werror -Wno-deprecated -Wno-unused-function ")
set(LIB
        xzmjx
        pthread
        yaml-cpp
        dl
        ${OPENSSL_LIBRARIES})

xzmjx_add_executable(test_log "test_log.cpp" xzmjx "${LIB}")

#xzmjx_add_executable(test_yaml test_yaml.cpp xzmjx ${LIB})

xzmjx_add_executable(test_thread  "test_thread.cpp" xzmjx "${LIB}")

xzmjx_add_executable(test_sharedptr  "test_sharedptr.cpp" xzmjx "${LIB}")

xzmjx_add_executable(test_context  "test_context.cpp" xzmjx "${LIB}")

xzmjx_add_executable(test_fiber  "test_fiber.cpp" xzmjx "${LIB}")

xzmjx_add_executable(test_cond  "test_cond.cpp" xzmjx "${LIB}")

xzmjx_add_executable(test_scheduler  "test_scheduler.cpp" xzmjx "${LIB}")
xzmjx_add_executable(test_iomanager  "test_iomanager.cpp" xzmjx "${LIB}")

xzmjx_add_executable(test_timer  "test_timer.cpp" xzmjx "${LIB}")
xzmjx_add_executable(test_hook  "test_hook.cpp" xzmjx "${LIB}")
xzmjx_add_executable(test_address  "test_address.cpp" xzmjx "${LIB}")
xzmjx_add_executable(test_socket  "test_socket.cpp" xzmjx "${LIB}")
xzmjx_add_executable(test_bytearray  "test_bytearray.cpp" xzmjx "${LIB}")
xzmjx_add_executable(test_tcpserver  "test_tcpserver.cpp" xzmjx "${LIB}")
xzmjx_add_executable(test_echo  "test_echo.cpp" xzmjx "${LIB}")
xzmjx_add_executable(test_http_parser  "test_http_parser.cpp" xzmjx "${LIB}")
xzmjx_add_executable(test_http_server  "test_http_server.cpp" xzmjx "${LIB}")
xzmjx_add_executable(test_daemon  "test_daemon.cpp" xzmjx "${LIB}")
xzmjx_add_executable(test_env  "test_env.cpp" xzmjx "${LIB}")


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
add_definitions(-Wno-builtin-macro-redefined)
force_redefine_file_macro_for_sources(xzmjx)
