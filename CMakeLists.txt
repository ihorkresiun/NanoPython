cmake_minimum_required(VERSION 3.10)

project(NanoPython)

include_directories(inc)

add_executable(NanoPython src/main.c src/ast.c src/eval.c src/parser.c src/token.c)