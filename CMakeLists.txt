cmake_minimum_required(VERSION 3.10)

project(my_project)

include_directories(inc)

add_executable(my_project src/main.c src/eval.c src/parser.c src/token.c)