cmake_minimum_required(VERSION 3.10)

project(NanoPython)

# Common source files shared across targets
set(COMMON_SOURCES
    src/ast.c
    src/compiler.c
    src/disasm.c
    src/hashmap.c
    src/lexer.c
    src/parser.c
    src/vars.c
)

# VM-specific sources
set(VM_SOURCES
    src/vm/gc.c
    src/vm/intern_string.c
    src/vm/native_func.c
    src/vm/vm.c
    src/vm/vm_objects.c
)

# Combined executable (compiler + VM)
add_executable(NanoPython
    ${COMMON_SOURCES}
    ${VM_SOURCES}
    src/main.c
)
target_include_directories(NanoPython PRIVATE inc inc/vm)
target_link_libraries(NanoPython m)

# Compiler-only executable
add_executable(NanoPythonCompiler
    ${COMMON_SOURCES}
    src/bytecode.c
    src/main_compiler.c
)
target_include_directories(NanoPythonCompiler PRIVATE inc inc/vm)
target_link_libraries(NanoPythonCompiler m)

# VM-only executable
add_executable(NanoPythonVM
    ${VM_SOURCES}
    src/hashmap.c
    src/vars.c
    src/bytecode.c
    src/main_vm.c
)

target_include_directories(NanoPythonVM PRIVATE inc inc/vm)
target_link_libraries(NanoPythonVM m)

add_executable(NanoPythonDisasm
    ${COMMON_SOURCES}
    src/bytecode.c
    src/main_disasm.c
)

target_include_directories(NanoPythonDisasm PRIVATE inc inc/vm)
target_link_libraries(NanoPythonDisasm m)