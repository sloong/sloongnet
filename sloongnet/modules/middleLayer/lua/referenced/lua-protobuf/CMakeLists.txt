cmake_minimum_required(VERSION 3.2)

project(pb)

message(WorkDir:${CMAKE_CURRENT_LIST_DIR}) 

include_directories(
    "$(PROJECT_BINARY_DIR)"
    "/usr/include/lua5.3"
)


#允许gdb调试
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -g -ggdb -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -DNODEBUG -DRELASE")

add_definitions(-DLUA_USE_READLINE)

#当前目录下所有源文件赋给SRC_LIST
aux_source_directory(. SRC_LIST)
list(APPEND SRC_LIST ${MODULE_SRC_LIST})

#生成可执行文件
add_library(${PROJECT_NAME} SHARED ${SRC_LIST}) 