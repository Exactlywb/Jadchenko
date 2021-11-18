#
# project: stack implementation
# author: Frolov Daniil
# author url: github.com/Exactlywb
#

cmake_minimum_required (VERSION 3.8)

project ("Stack")

set (EXEC_FILE "stack")

set (SOURCES

    main.c
    source/stack.c

)

add_executable (${EXEC_FILE} ${SOURCES})

target_include_directories (${EXEC_FILE} PRIVATE include)

add_compile_options(

    -Werror

    -Wall
    -Wextra
    -Wpedantic

    -Wdiv-by-zero

    -Wfloat-equal
    -Wbool-compire

    -Wsign-promo
    -Wsign-conversion
    -Wdouble-promotion

    -O2

)

