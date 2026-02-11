# NanoPython Test Suite

This directory contains general test cases for the NanoPython compiler and VM.

## Test Files

- **test_arithmetic.py** - Basic arithmetic operations (addition, subtraction, multiplication, division)
- **test_conditionals.py** - If/else statements and comparison operators
- **test_loops.py** - While loops, for loops, break, and continue statements
- **test_functions.py** - Function definitions, parameters, return values, and recursion
- **test_data_structures.py** - Lists, tuples, sets, and dictionaries
- **test_classes.py** - Class definitions, objects, methods, and inheritance
- **test_strings.py** - String operations and assignments
- **test_scope.py** - Variable scope and nested functions
- **test_edge_cases.py** - Edge cases like empty lists, zero values, negative numbers
- **test_comprehensive.py** - Complex test combining multiple features

## Running Tests

### Run All Tests

```bash
cd test
./run_tests.sh
```

### Run Individual Test

```bash
cd build
./NanoPythonCompiler ../test/test_arithmetic.py test.bcd
./NanoPythonVM test.bcd
```

### Examine Bytecode

```bash
cd build
./NanoPythonCompiler ../test/test_functions.py test.bcd
./NanoPythonDisasm test.bcd test.asm
cat test.asm
```

## Test Coverage

These tests cover:
- ✓ Basic arithmetic and expressions
- ✓ Control flow (if/else, while, for)
- ✓ Functions (definition, calls, recursion, return values)
- ✓ Data structures (lists, tuples, sets, dicts, indexing)
- ✓ Classes and objects (methods, attributes, inheritance)
- ✓ Variable scope (global, local, nested)
- ✓ String operations
- ✓ Edge cases and special scenarios

## Adding New Tests

1. Create a new file `test_<name>.py` in this directory
2. Add descriptive comments and print statements
3. Run `./run_tests.sh` to verify it works

## Expected Output

All tests should:
- Compile successfully without errors
- Run without segmentation faults
- Produce expected output
- Return exit code 0
