cmake_minimum_required(VERSION 3.2)

project(core)

message(WorkDir:${CMAKE_CURRENT_LIST_DIR}) 

include_directories(
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/../../"
	"${CMAKE_CURRENT_LIST_DIR}/../../referenced/libuniv"
    "/usr/include/glib-2.0" 
    "/usr/lib/x86_64-linux-gnu/glib-2.0/include"
)

link_directories(
    "${CMAKE_CURRENT_LIST_DIR}/referenced/libuniv"
)

# 设置动态库输出路径
SET(LIBRARY_OUTPUT_PATH "../")

#允许gdb调试
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -g -pg -ggdb -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -DNODEBUG -DRELASE")

add_definitions(-DLUA_USE_READLINE)

# 代码覆盖信息
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftest-coverage -fprofile-arcs")

# 废弃API检测
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated")

# 警告信息选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-switch -Wno-return-type -Wno-misleading-indentation")

# C++ 版本
set(CMAKE_CXX_FLAGS "-std=c++17 ${CMAKE_CXX_FLAGS}")

#当前目录下所有源文件赋给SRC_LIST
aux_source_directory(. SRC_LIST)
list(APPEND SRC_LIST ${MODULE_SRC_LIST})

# 编译Protobuf
find_package(Protobuf REQUIRED)
FILE(GLOB protofiles "${CMAKE_CURRENT_LIST_DIR}/protocol/*.proto")
protobuf_generate_cpp_ex(${CMAKE_CURRENT_LIST_DIR}/protocol PROTOSRCS PROTOHDRS ${protofiles})
list(APPEND SRC_LIST ${PROTOSRCS})

#生成动态链接库
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRC_LIST})

#添加链接库
target_link_libraries(${PROJECT_NAME} m)
target_link_libraries(${PROJECT_NAME} dl)
target_link_libraries(${PROJECT_NAME} uuid)
target_link_libraries(${PROJECT_NAME} pthread)
target_link_libraries(${PROJECT_NAME} ssl)
target_link_libraries(${PROJECT_NAME} crypto)
target_link_libraries(${PROJECT_NAME} lua5.3)
target_link_libraries(${PROJECT_NAME} glib-2.0)
target_link_libraries(${PROJECT_NAME} univ)
target_link_libraries(${PROJECT_NAME} sqlite3)
target_link_libraries(${PROJECT_NAME} protobuf)
target_link_libraries(${PROJECT_NAME} boost_serialization)
target_link_libraries(${PROJECT_NAME} jsoncpp)
