# Compiler, Flags, Directory Name and Executable Name
CC= g++
CFLAGS= -std=c++11 -Wall -Wextra -I../src
CFLAGS_COV= -std=c++11 -Wall -Wextra -I../src --coverage -fprofile-arcs -ftest-coverage
ODIR= ./obj
COVDIR= ./coverage_report

# Test executables
TEST_LEGACY= runTestsLegacy
TEST_CLEAN= runTestsClean
TEST_COV= runTestsCoverage

all: $(ODIR) $(TEST_LEGACY) $(TEST_CLEAN)

$(ODIR):
	@mkdir -p $(ODIR)

# Legacy tests (original code)
$(TEST_LEGACY): testPoint.cpp catch.hpp
	$(CC) -std=c++11 -Wall -Wextra testPoint.cpp -o $(TEST_LEGACY)

# Clean architecture tests
$(TEST_CLEAN): testCleanArch.cpp catch.hpp
	$(CC) $(CFLAGS) testCleanArch.cpp -o $(TEST_CLEAN)

# Clean architecture tests with coverage
$(TEST_COV): testCleanArch.cpp catch.hpp
	@mkdir -p $(COVDIR)
	$(CC) $(CFLAGS_COV) testCleanArch.cpp -o $(TEST_COV)

# Run all tests
test: all
	@echo "=== Running Legacy Tests ==="
	./$(TEST_LEGACY)
	@echo ""
	@echo "=== Running Clean Architecture Tests ==="
	./$(TEST_CLEAN)

# Run only clean architecture tests
test-clean: $(TEST_CLEAN)
	./$(TEST_CLEAN)

# Run only legacy tests
test-legacy: $(TEST_LEGACY)
	./$(TEST_LEGACY)

# Run tests with verbose output
test-verbose: all
	./$(TEST_LEGACY) -s
	./$(TEST_CLEAN) -s

# Run tests with code coverage (requires lcov)
coverage: $(TEST_COV)
	@mkdir -p $(COVDIR)
	@echo "=== Running Tests with Coverage ==="
	./$(TEST_COV)
	@echo ""
	@echo "=== Generating Coverage Report ==="
	lcov --capture --directory . --output-file $(COVDIR)/coverage.info --ignore-errors source 2>/dev/null || true
	lcov --remove $(COVDIR)/coverage.info '/usr/*' '*/catch.hpp' --output-file $(COVDIR)/coverage_filtered.info 2>/dev/null || true
	@if [ -f $(COVDIR)/coverage_filtered.info ]; then \
		genhtml $(COVDIR)/coverage_filtered.info --output-directory $(COVDIR)/html 2>/dev/null && \
		echo "Coverage report generated at: $(COVDIR)/html/index.html"; \
	else \
		echo "Coverage report generation requires lcov. Install with: sudo apt install lcov"; \
	fi

# Show coverage summary
coverage-summary: $(TEST_COV)
	@./$(TEST_COV) > /dev/null 2>&1 || true
	@gcov testCleanArch.cpp 2>/dev/null | grep -A1 "File '../src" | head -20 || echo "Install gcov for coverage summary"

# Count lines of code (requires cloc or sloccount)
loc:
	@echo "=== Lines of Code (Clean Architecture) ==="
	@if command -v cloc > /dev/null 2>&1; then \
		cloc ../src --by-file --quiet; \
	elif command -v sloccount > /dev/null 2>&1; then \
		sloccount ../src 2>/dev/null; \
	else \
		echo "Install cloc for detailed stats: sudo apt install cloc"; \
		echo ""; \
		echo "Quick count (wc -l):"; \
		find ../src -name "*.hpp" -o -name "*.cpp" | xargs wc -l | tail -1; \
	fi
	@echo ""
	@echo "=== Lines of Code (Full Project) ==="
	@if command -v cloc > /dev/null 2>&1; then \
		cloc .. --exclude-dir=obj,bin,tests,node_modules,.git --quiet; \
	else \
		echo "Source files:"; \
		find .. -name "*.hpp" -o -name "*.cpp" -o -name "*.js" | grep -v node_modules | grep -v obj | xargs wc -l 2>/dev/null | tail -1; \
	fi

# Project statistics
stats: loc
	@echo ""
	@echo "=== Test Statistics ==="
	@echo "Legacy tests: $$(./$(TEST_LEGACY) --list-tests 2>/dev/null | wc -l) test cases"
	@echo "Clean tests:  $$(./$(TEST_CLEAN) --list-tests 2>/dev/null | wc -l) test cases"

# Delete objects and executables
clean:
	rm -rf $(ODIR) $(COVDIR) $(TEST_LEGACY) $(TEST_CLEAN) $(TEST_COV) *.gcno *.gcda *.gcov

.PHONY: all test test-clean test-legacy test-verbose coverage coverage-summary loc stats clean