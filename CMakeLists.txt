cmake_minimum_required (VERSION 3.0.2)

project (machineid)

add_library (machineid machineid.c)

add_executable (main main.c)

target_link_libraries (main machineid)

target_compile_options (machineid PRIVATE
    -std=c89
    -pedantic
    -Wextra
)