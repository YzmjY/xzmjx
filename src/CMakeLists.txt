include_directories(${PROJECT_SOURCE_DIR}/include)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -g -std=c++14  -Wall -Werror -Wno-deprecated -Wno-unused-function ")

set(LIB_SRC
        address.cpp
        log.cpp
        util.cpp
        #config.cpp
        thread.cpp
        mutex.cpp
        fiber.cpp
        scheduler.cpp
        iomanager.cpp
        timer.cpp
        hook.cpp
        fdmanager.cpp)
set(LIB_LIB pthread yaml-cpp dl)

add_library(xzmjx STATIC ${LIB_SRC})
target_link_libraries(xzmjx ${LIB_LIB})



set(LIBRARY_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
