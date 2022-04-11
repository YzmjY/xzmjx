include_directories(${PROJECT_SOURCE_DIR}/include)
link_directories(${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -g -std=c++14  -Wall -Werror -Wno-deprecated -Wno-unused-function ")
set(LIB xzmjx yaml-cpp)

xzmjx_add_executable(test_log test_log.cpp xzmjx ${LIB})

#xzmjx_add_executable(test_yaml test_yaml.cpp xzmjx ${LIB})

xzmjx_add_executable(test_thread  test_thread.cpp xzmjx ${LIB})

xzmjx_add_executable(test_sharedptr  test_sharedptr.cpp xzmjx ${LIB})

xzmjx_add_executable(test_context  test_context.cpp xzmjx ${LIB})

xzmjx_add_executable(test_fiber  test_fiber.cpp xzmjx ${LIB})

xzmjx_add_executable(test_cond  test_cond.cpp xzmjx ${LIB})

xzmjx_add_executable(test_scheduler  test_scheduler.cpp xzmjx ${LIB})
xzmjx_add_executable(test_iomanager  test_iomanager.cpp xzmjx ${LIB})

xzmjx_add_executable(test_timer  test_timer.cpp xzmjx ${LIB})
xzmjx_add_executable(test_hook  test_hook.cpp xzmjx ${LIB})


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)