#!/bin/bash

# Test runner for NanoPython
# Compiles and runs all test files in the test directory

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if build directory exists
if [ ! -d "../build" ]; then
    echo -e "${RED}Error: build directory not found${NC}"
    echo "Please run CMake and build the project first"
    exit 1
fi

# Paths
BUILD_DIR="../build"
COMPILER="$BUILD_DIR/NanoPythonCompiler"
VM="$BUILD_DIR/NanoPythonVM"
BYTECODE="test.bcd"

# Check if executables exist
if [ ! -f "$COMPILER" ] || [ ! -f "$VM" ]; then
    echo -e "${RED}Error: Compiler or VM not found${NC}"
    echo "Please build the project first"
    exit 1
fi

echo "======================================="
echo "   NanoPython Test Suite"
echo "======================================="
echo

# Counter for statistics
total=0
passed=0
failed=0

# Run each test file
for test_file in test_*.py; do
    if [ -f "$test_file" ]; then
        total=$((total + 1))
        echo -e "${YELLOW}Running:${NC} $test_file"
        echo "---------------------------------------"
        
        # Compile
        $COMPILER "$test_file" "$BYTECODE" 2>&1 > /dev/null
        compile_status=$?
        
        if [ $compile_status -ne 0 ]; then
            echo -e "${RED}✗ FAILED${NC} - Compilation error"
            failed=$((failed + 1))
            echo
            continue
        fi
        
        # Run
        $VM "$BYTECODE" 2>&1
        run_status=$?
        
        if [ $run_status -eq 0 ]; then
            echo -e "${GREEN}✓ PASSED${NC}"
            passed=$((passed + 1))
        else:
            echo -e "${RED}✗ FAILED${NC} - Runtime error (exit code: $run_status)"
            failed=$((failed + 1))
        fi
        
        echo
    fi
done

# Clean up
rm -f "$BYTECODE"

# Print summary
echo "======================================="
echo "   Test Summary"
echo "======================================="
echo -e "Total:  $total"
echo -e "${GREEN}Passed: $passed${NC}"
if [ $failed -gt 0 ]; then
    echo -e "${RED}Failed: $failed${NC}"
else
    echo -e "Failed: $failed"
fi
echo

# Exit with appropriate code
if [ $failed -gt 0 ]; then
    exit 1
else
    exit 0
fi
