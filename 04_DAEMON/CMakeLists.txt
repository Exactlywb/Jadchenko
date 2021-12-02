#
# project:  Copy daemon
# author:   Frolov Daniil (aka exactlywb)
# url:      github.com/exactlywb
#

cmake_minimum_required (VERSION 3.8)
project ("Daemon")

set (EXEC_FILE "daemon")
set (SOURCES source/main.c source/copyInstr.c)

add_executable (${EXEC_FILE} ${SOURCES})
target_include_directories (${EXEC_FILE} PRIVATE include)

add_compile_options (

    -Werror
    
    -Wall
    -Wextra
    -Wpedantic

    -O0

)

