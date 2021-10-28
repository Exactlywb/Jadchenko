cmake_minimum_required (VERSION 3.8)

project ("Cane" LANGUAGES C)

set(FILE_NAME "main")

set(HEADERS Cane.h)

set(SOURCES
        main.c
        Cane.c
)

add_executable(${FILE_NAME} ${SOURCES})

add_compile_options(-Werror -Wall -Wextra -Wpedantic)
