cmake_minimum_required(VERSION 3.2)

project(gateway)

message(WorkDir:${CMAKE_CURRENT_LIST_DIR}) 

#当前目录下所有源文件赋给SRC_LIST
aux_source_directory(. SRC_LIST)
list(APPEND SRC_LIST ${MODULE_SRC_LIST})

# 编译Protobuf
find_package(Protobuf REQUIRED)
FILE(GLOB protofiles "${CMAKE_CURRENT_LIST_DIR}/protocol/*.proto")
protobuf_generate_cpp_ex(${CMAKE_CURRENT_LIST_DIR}/protocol PROTOSRCS PROTOHDRS ${protofiles})
list(APPEND SRC_LIST ${PROTOSRCS})

#生成可执行文件
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRC_LIST})
