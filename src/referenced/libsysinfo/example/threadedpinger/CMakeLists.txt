

cmake_minimum_required(VERSION 3.0)
project(threadedpinger)

set(CMAKE_CXX_STANDARD 17)

add_executable(threadedpinger
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
        )

#target_compile_options(threadedpinger PRIVATE ${COMPILER_SETTINGS})
target_link_libraries(threadedpinger linuxmonitoring)

