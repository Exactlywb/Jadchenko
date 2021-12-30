#
# project:  Copy chert (daemon remake)
# author:   Frolov Daniil (aka exactlywb)
# url:      github.com/exactlywb
#

cmake_minimum_required (VERSION 3.8)
project ("Daemon")

set (EXEC_FILE "daemon")
set (SOURCES 
    source/main.c 
    source/copyTools/copy.c 
    source/daemon/daemon.c
    source/inotifyR/inotifyR.c
    source/common.c
)

add_executable (${EXEC_FILE} ${SOURCES})

target_include_directories (${EXEC_FILE} PRIVATE include)
target_include_directories (${EXEC_FILE} PRIVATE include/daemon)
target_include_directories (${EXEC_FILE} PRIVATE include/copyTools)
target_include_directories (${EXEC_FILE} PRIVATE include/inotifyR)

add_compile_options (

    -Werror
    
    -Wall
    -Wextra
    -Wpedantic

    -O0

)