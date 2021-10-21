cmake_minimum_required(VERSION 3.0)
project(LinuxMonitoring LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(COMPILER_SETTINGS
    -Wall
    -pedantic
    -Wextra
    -g
    -ggdb
    -Werror
    -Wshadow
    -Wold-style-cast
    -Wsign-conversion
    -Woverloaded-virtual
    -Wuseless-cast
    -Wsign-promo
    -Wstrict-null-sentinel
    -Wno-unused
    -Wstrict-overflow=1
    -Wlogical-op
    -Winit-self
    -Wformat=2
    -Wcast-align
    -Wcast-qual
    -Wdisabled-optimization
    -Wmissing-include-dirs
    -Wno-format-nonliteral
    )


option(USE_THIRDPARTY_EXAMPLES "Use thirdparty examples" OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")

if (USE_THIRDPARTY_EXAMPLES)
    include_directories(thirdparty/cxxopts/include)
    include_directories(thirdparty/Catch2/single_include)
    include_directories(thirdparty/json/single_include/nlohmann)
    include_directories(thirdparty/json/single_include)
    include_directories(thirdparty/uWebSockets/uSockets/src)
    include_directories(thirdparty/uWebSockets/src)
    link_directories(${CMAKE_CURRENT_LIST_DIR}/thirdparty/uWebSockets/uSockets)
    include_directories(${CMAKE_CURRENT_LIST_DIR}/thirdparty/uWebSockets/uSockets)
endif()


include(${CMAKE_CURRENT_LIST_DIR}/lib/CMakeLists.txt)
if (USE_THIRDPARTY_EXAMPLES)
    include(${CMAKE_CURRENT_LIST_DIR}/tests/CMakeLists.txt)
    include(${CMAKE_CURRENT_LIST_DIR}/example/LinuxSystemMonitor_WebsocketService/CMakeLists.txt)
    include(${CMAKE_CURRENT_LIST_DIR}/example/threadedpinger/CMakeLists.txt)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/example/simplemonitor/CMakeLists.txt)
