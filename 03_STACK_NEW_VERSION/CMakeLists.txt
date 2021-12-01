#
# project:  shared memory stack
# author:   Frolov Daniil (aka exactlywb)
# url:      github.com/Exactlywb
#

cmake_minimum_required (VERSION 3.8)

project ("Stack")

set (EXEC_FILE "stack")

set (SOURCES main.c sources/stack.c)

add_executable (${EXEC_FILE} ${SOURCES})
target_include_directories (${EXEC_FILE} PRIVATE include)

add_compile_options (

    -Werror
    
    -Wall
    -Wextra
    -Wpedantic

    -O0

)

