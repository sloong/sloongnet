cmake_minimum_required(VERSION 3.16)

project(sloongnet)

message(WorkDir:${CMAKE_CURRENT_LIST_DIR}) 
include(${CMAKE_CURRENT_LIST_DIR}/protobuf_ex.cmake)

include_directories(
    "${CMAKE_CURRENT_LIST_DIR}"
	"${CMAKE_CURRENT_LIST_DIR}/referenced/libuniv/include"
    "${CMAKE_CURRENT_LIST_DIR}/referenced"
    "${CMAKE_CURRENT_LIST_DIR}/modules/core"
)

link_directories(
    "${CMAKE_CURRENT_LIST_DIR}/referenced/libuniv"
    "${CMAKE_CURRENT_LIST_DIR}/modules"
)

# 设置动态库输出路径
SET(LIBRARY_OUTPUT_PATH "../")

#允许gdb调试
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -g -ggdb -DDEBUG -D_GLIBCXX_DEBUG")
# the clang with O2/O3 will happend protobuf error. so in here the release set to O1
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O1 -DNODEBUG -DRELASE")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_definitions(-DLUA_USE_READLINE)

# 警告信息选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-switch -Wno-return-type-c-linkage")

# C++ 版本
set(CMAKE_CXX_FLAGS "-std=c++17 ${CMAKE_CXX_FLAGS}")

# 添加module
add_subdirectory(modules/core)
add_subdirectory(modules/manager)
add_subdirectory(modules/firewall)
add_subdirectory(modules/gateway)
add_subdirectory(modules/datacenter)
add_subdirectory(modules/filecenter)
add_subdirectory(modules/middleLayer/lua)

#当前目录下所有源文件赋给SRC_LIST
aux_source_directory(. SRC_LIST)
list(APPEND SRC_LIST ${MODULE_SRC_LIST})

# 编译Protobuf
find_package(Protobuf REQUIRED)
FILE(GLOB protofiles "${CMAKE_CURRENT_LIST_DIR}/protocol/*.proto")
protobuf_generate_cpp_ex(${CMAKE_CURRENT_LIST_DIR}/protocol PROTOSRCS PROTOHDRS ${protofiles})
list(APPEND SRC_LIST ${PROTOSRCS})

#生成可执行文件
add_executable(${PROJECT_NAME} ${SRC_LIST}) 

target_compile_options(${PROJECT_NAME} PRIVATE -fpie)

# http://sourceware.org/binutils/docs/ld/Options.html
# -E/--export-dynamic:  add all symbols to the dynamic symbol table.
# -rpath: Add a directory to the runtime library search path.
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,-rpath=.")
add_link_options("-Wl,-E")

#添加链接库
target_link_libraries(${PROJECT_NAME} 
        m
        dl
        protobuf
        jsoncpp
        core
        manager)
