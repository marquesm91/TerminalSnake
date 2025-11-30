# Compiler, Flags, Directory Name and Executable Name
CC= g++
CFLAGS= -std=c++11 -Wall -Wextra
ODIR= ./obj

# Test executables
TEST_POINT= test_point
TEST_TERMINAL= test_terminal
TEST_WEB_CURSES= test_web_curses
TEST_ALL= test_all

all: $(ODIR) $(TEST_POINT) $(TEST_TERMINAL) $(TEST_WEB_CURSES) $(TEST_ALL)

$(ODIR):
	@mkdir -p $(ODIR)

# Individual test - Point
$(TEST_POINT): testPoint.cpp catch.hpp
	$(CC) $(CFLAGS) testPoint.cpp -o $(TEST_POINT)

# Individual test - Terminal
$(TEST_TERMINAL): testTerminal.cpp catch.hpp
	$(CC) $(CFLAGS) testTerminal.cpp -o $(TEST_TERMINAL)

# Individual test - Web Curses
$(TEST_WEB_CURSES): testWebCurses.cpp catch.hpp
	$(CC) $(CFLAGS) -I. -Imocks testWebCurses.cpp -o $(TEST_WEB_CURSES)

# Individual test - Game Mechanics
TEST_MECHANICS= test_mechanics
$(TEST_MECHANICS): testGameMechanics.cpp catch.hpp
	$(CC) $(CFLAGS) -I../libs testGameMechanics.cpp -o $(TEST_MECHANICS) -lncurses

# Combined test runner (runs all tests)
$(TEST_ALL): $(TEST_POINT) $(TEST_TERMINAL) $(TEST_WEB_CURSES) $(TEST_MECHANICS)
	@echo "Combined test runner created"
	@touch $(TEST_ALL)

# Run all tests
test: all
	@echo "================================"
	@echo "Running Point Tests..."
	@echo "================================"
	./$(TEST_POINT)
	@echo ""
	@echo "================================"
	@echo "Running Terminal Tests..."
	@echo "================================"
	./$(TEST_TERMINAL)
	@echo ""
	@echo "================================"
	@echo "Running Web Curses Tests..."
	@echo "================================"
	@./$(TEST_WEB_CURSES)
	@echo ""
	@echo "================================"
	@echo "Running Game Mechanics Tests..."
	@echo "================================"
	@./$(TEST_MECHANICS)

# Run only point tests
test-point: $(TEST_POINT)
	./$(TEST_POINT)

# Run only terminal tests
test-terminal: $(TEST_TERMINAL)
	./$(TEST_TERMINAL)

# Verbose test output
test-verbose: all
	./$(TEST_POINT) -v
	./$(TEST_TERMINAL) -v

# Delete objects and executables
clean:
	rm -rf $(ODIR) $(TEST_POINT) $(TEST_TERMINAL) $(TEST_ALL)

# Coverage target
coverage:
	$(CC) $(CFLAGS) --coverage testPoint.cpp -o test_point_cov
	$(CC) $(CFLAGS) --coverage testTerminal.cpp -o test_terminal_cov
	$(CC) $(CFLAGS) -I. -Imocks --coverage testWebCurses.cpp -o test_web_curses_cov
	./test_point_cov
	./test_terminal_cov
	./test_web_curses_cov
	lcov --capture --directory . --output-file coverage.info
	lcov --remove coverage.info '/usr/*' '*/tests/*' '*/mocks/*' --ignore-errors unused --output-file coverage.info
	genhtml coverage.info --output-directory coverage_report
	@echo "Coverage report generated in tests/coverage_report/index.html"

.PHONY: all test test-point test-terminal test-verbose clean coverage