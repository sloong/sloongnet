cmake_minimum_required(VERSION 3.2)

project(sloongnet)

message(WorkDir:${CMAKE_CURRENT_LIST_DIR}) 
include(${CMAKE_CURRENT_LIST_DIR}/protobuf_ex.cmake)

include_directories(
    "${CMAKE_CURRENT_LIST_DIR}"
	"${CMAKE_CURRENT_LIST_DIR}/referenced/libuniv/include"
    "${CMAKE_CURRENT_LIST_DIR}/modules/core"
)

link_directories(
    "${CMAKE_CURRENT_LIST_DIR}/referenced/libuniv"
    "${CMAKE_CURRENT_LIST_DIR}/modules"
)

# 设置动态库输出路径
SET(LIBRARY_OUTPUT_PATH "../")

#允许gdb调试
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -g -ggdb -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -DNODEBUG -DRELASE")

add_definitions(-DLUA_USE_READLINE)

# 警告信息选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-switch")

# C++ 版本
set(CMAKE_CXX_FLAGS "-std=c++17 ${CMAKE_CXX_FLAGS}")

# 添加module
add_subdirectory(modules/core)
add_subdirectory(modules/manager)
add_subdirectory(modules/firewall)
add_subdirectory(modules/gateway)
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
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,-rpath,./")

#添加链接库
target_link_libraries(${PROJECT_NAME} m)
target_link_libraries(${PROJECT_NAME} dl)
target_link_libraries(${PROJECT_NAME} univ)
target_link_libraries(${PROJECT_NAME} protobuf)
target_link_libraries(${PROJECT_NAME} jsoncpp)
target_link_libraries(${PROJECT_NAME} core)
