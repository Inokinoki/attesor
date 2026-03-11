#!/bin/bash
# ============================================================================
# Rosetta 2 Binary Translator - Comprehensive Test Runner
# ============================================================================
#
# This script runs all Rosetta 2 test suites and provides a summary
# of the results.
#
# Usage: ./run_all_tests.sh
#
# ============================================================================

echo "╔════════════════════════════════════════════════════════════════╗"
echo "║     Rosetta 2 Binary Translator - Comprehensive Test Suite    ║"
echo "║     Testing x86_64 → ARM64 Translation                        ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test counters
TOTAL_TESTS=0
TOTAL_PASSED=0
TOTAL_FAILED=0
TOTAL_SKIPPED=0

# Array to track test results
declare -a TEST_NAMES=()
declare -a TEST_RESULTS=()

# Function to run a test
run_test() {
    local test_name=$1
    local test_command=$2

    echo -e "${BLUE}Running:${NC} $test_name"

    if eval "$test_command" > /tmp/test_output.log 2>&1; then
        echo -e "  ${GREEN}✓ PASSED${NC}"
        TOTAL_PASSED=$((TOTAL_PASSED + 1))
        TEST_RESULTS+=("PASS")

        # Extract test count from output
        local count=$(grep -oP "Total Tests:\s*\K\d+" /tmp/test_output.log | head -1)
        if [ ! -z "$count" ]; then
            echo "  Tests run: $count"
            TOTAL_TESTS=$((TOTAL_TESTS + count))
        fi
    else
        echo -e "  ${RED}✗ FAILED${NC}"
        TOTAL_FAILED=$((TOTAL_FAILED + 1))
        TEST_RESULTS+=("FAIL")
        echo "  Error output:"
        cat /tmp/test_output.log | head -20
    fi

    TEST_NAMES+=("$test_name")
    echo ""
}

# Run all test suites
echo "════════════════════════════════════════════════════════════════"
echo "Phase 1: Infrastructure Tests"
echo "════════════════════════════════════════════════════════════════"
echo ""

run_test "Basic Infrastructure Demo" "./rosetta_demo"
run_test "Comprehensive Infrastructure Tests" "./rosetta_demo_full"
run_test "End-to-End Translation Demo" "./test_e2e_translation"

echo "════════════════════════════════════════════════════════════════"
echo "Phase 2: Instruction Translation Tests"
echo "════════════════════════════════════════════════════════════════"
echo ""

run_test "Basic Instruction Translation" "./test_translator all 2>&1 | head -100"

echo "════════════════════════════════════════════════════════════════"
echo "Phase 3: Floating-Point and SIMD Tests"
echo "════════════════════════════════════════════════════════════════"
echo ""

run_test "FP/SIMD Instruction Tests" "./test_fp_simd all"

echo "════════════════════════════════════════════════════════════════"
echo "Phase 4: String and Advanced Instruction Tests"
echo "════════════════════════════════════════════════════════════════"
echo ""

run_test "String/Advanced Instruction Tests" "./test_string_advanced all"

# Print final summary
echo "════════════════════════════════════════════════════════════════"
echo "                         Test Summary"
echo "════════════════════════════════════════════════════════════════"
echo ""

echo "Test Suites Run: ${#TEST_NAMES[@]}"
echo ""

for i in "${!TEST_NAMES[@]}"; do
    if [ "${TEST_RESULTS[$i]}" = "PASS" ]; then
        echo -e "  ${GREEN}✓${NC} ${TEST_NAMES[$i]}"
    else
        echo -e "  ${RED}✗${NC} ${TEST_NAMES[$i]}"
    fi
done

echo ""
echo "════════════════════════════════════════════════════════════════"
echo ""

# Final status
if [ $TOTAL_FAILED -eq 0 ]; then
    echo -e "${GREEN}╔════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║                   ALL TESTS PASSED ✓                          ║${NC}"
    echo -e "${GREEN}╠════════════════════════════════════════════════════════════════╣${NC}"
    echo -e "${GREEN}║  Test Suites:      ${#TEST_NAMES[@]}                                        ║${NC}"
    echo -e "${GREEN}║  Test Cases:       $TOTAL_TESTS                                        ║${NC}"
    echo -e "${GREEN}║  Passed:           $TOTAL_PASSED  ✓                                ║${NC}"
    echo -e "${GREEN}║  Failed:           $TOTAL_FAILED  ✗                                ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo "The Rosetta 2 binary translator is ready for production use!"
    echo ""
    exit 0
else
    echo -e "${RED}╔════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${RED}║                   SOME TESTS FAILED ✗                        ║${NC}"
    echo -e "${RED}╠════════════════════════════════════════════════════════════════╣${NC}"
    echo -e "${RED}║  Test Suites:      ${#TEST_NAMES[@]}                                        ║${NC}"
    echo -e "${RED}║  Test Cases:       $TOTAL_TESTS                                        ║${NC}"
    echo -e "${RED}║  Passed:           $TOTAL_PASSED  ✓                                ║${NC}"
    echo -e "${RED}║  Failed:           $TOTAL_FAILED  ✗                                ║${NC}"
    echo -e "${RED}╚════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo "Please review the error output above for details."
    echo ""
    exit 1
fi
