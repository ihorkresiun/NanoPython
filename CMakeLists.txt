cmake_minimum_required(VERSION 3.10)

project(NanoPython)

# include_directories(inc inc/vm)

add_executable(NanoPython src/main.c src/ast.c src/compiler.c src/disasm.c src/gc.c src/hashmap.c src/intern_string.c src/lexer.c src/native_func.c src/parser.c src/vars.c src/vm/vm.c src/vm/vm_objects.c)

target_include_directories(NanoPython PRIVATE inc inc/vm)
target_link_libraries(NanoPython m)