cmake_minimum_required(VERSION 3.10)

project(NanoPython)

# Combined executable (compiler + VM)
add_executable(NanoPython
    src/ast.c
    src/bytecode.c
    src/compiler.c
    src/hashmap.c
    src/lexer.c
    src/main.c
    src/parser.c
    src/vars.c
    src/vm/gc.c
    src/vm/intern_string.c
    src/vm/native_func.c
    src/vm/vm.c
    src/vm/vm_objects.c
)
target_include_directories(NanoPython PRIVATE inc inc/vm)
target_link_libraries(NanoPython m)

# Compiler-only executable
add_executable(NanoPythonCompiler
    src/ast.c
    src/bytecode.c
    src/compiler.c
    src/hashmap.c
    src/lexer.c
    src/main_compiler.c
    src/parser.c
    src/vars.c
)
target_include_directories(NanoPythonCompiler PRIVATE inc inc/vm)
target_link_libraries(NanoPythonCompiler m)

# VM-only executable
add_executable(NanoPythonVM
    src/bytecode.c
    src/hashmap.c
    src/main_vm.c
    src/vars.c
    src/vm/gc.c
    src/vm/intern_string.c
    src/vm/native_func.c
    src/vm/vm.c
    src/vm/vm_objects.c
)

target_include_directories(NanoPythonVM PRIVATE inc inc/vm)
target_link_libraries(NanoPythonVM m)

add_executable(NanoPythonDisasm
    src/bytecode.c
    src/main_disasm.c
)

target_include_directories(NanoPythonDisasm PRIVATE inc inc/vm)
target_link_libraries(NanoPythonDisasm m)