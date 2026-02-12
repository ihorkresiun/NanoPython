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
NANOPYTHON="$BUILD_DIR/NanoPython"

# Check if executable exists
if [ ! -f "$NANOPYTHON" ]; then
    echo -e "${RED}Error: NanoPython executable not found${NC}"
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
        
        # Run with integrated NanoPython
        $NANOPYTHON "$test_file" 2>&1
        run_status=$?
        
        if [ $run_status -eq 0 ]; then
            echo -e "${GREEN}✓ PASSED${NC}"
            passed=$((passed + 1))
        else
            echo -e "${RED}✗ FAILED${NC} - Runtime error (exit code: $run_status)"
            failed=$((failed + 1))
        fi
        
        echo
    fi
done

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
if [ $failed -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed${NC}"
    exit 1
fi