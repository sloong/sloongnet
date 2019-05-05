cmake_minimum_required(VERSION 3.2)

project(sloongnet_control)

include_directories(
    "$(PROJECT_BINARY_DIR)"
    "$(PROJECT_BINARY_DIR)../"
    "$(PROJECT_BINARY_DIR)../comm/interface"
    "$(PROJECT_BINARY_DIR)../comm/events"
    "$(PROJECT_BINARY_DIR)../comm/modules"
    "$(PROJECT_BINARY_DIR)../comm/Message"
    "$(PROJECT_BINARY_DIR)../reference"
    "/usr/include/glib-2.0"
	# ubuntu path
    "/usr/lib/x86_64-linux-gnu/glib-2.0/include"
	# centos path
	"/usr/lib64/glib-2.0/include"
)

link_directories(
    "/usr/local/lib/sloong"
	#centos path
	"/usr/lib64/mysql"
)

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
#AUX_SOURCE_DIRECTORY(. SRC_LIST) 
file(GLOB_RECURSE COMM_SRC_LIST "../comm/*.c*")
file(GLOB_RECURSE SRC_LIST "*.c*")
list(APPEND SRC_LIST ${COMM_SRC_LIST})

#生成可执行文件
add_executable(${PROJECT_NAME} ${SRC_LIST}) 

#添加C++11编译选项,但是只有CMAKE 3.1支持
#target_compile_features(mmseg PRIVATE cxx_range_for)


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