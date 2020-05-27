cmake_minimum_required(VERSION 3.2)

project(luaMiddleLayer)

message(WorkDir:${CMAKE_CURRENT_LIST_DIR}) 

include_directories(
	"${CMAKE_CURRENT_LIST_DIR}"
	"/usr/include/luajit-2.1"
)

SET(LIBRARY_OUTPUT_PATH "../../")

#当前目录下所有源文件赋给SRC_LIST
aux_source_directory(. SRC_LIST)
list(APPEND SRC_LIST ${MODULE_SRC_LIST})

#生成可执行文件
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRC_LIST})

#添加链接库
target_link_libraries(${PROJECT_NAME} luajit-5.1)