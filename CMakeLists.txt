cmake_minimum_required(VERSION 3.13)
project(zadanie)

set(CMAKE_CXX_STANDARD 14)

add_executable(zadanie
        main.c
        lexer.c
        interpreter.c
        generator.c
        )