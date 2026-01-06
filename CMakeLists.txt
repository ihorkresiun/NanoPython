cmake_minimum_required(VERSION 3.10)

project(NanoPython)

# include_directories(inc)

add_executable(NanoPython src/main.c src/ast.c src/eval.c src/lexer.c src/parser.c)

target_include_directories(NanoPython PRIVATE inc)
target_link_libraries(NanoPython m)