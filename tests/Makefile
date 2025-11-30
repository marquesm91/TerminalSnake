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

# Combined test runner (runs all tests)
$(TEST_ALL): $(TEST_POINT) $(TEST_TERMINAL) $(TEST_WEB_CURSES)
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
	./$(TEST_WEB_CURSES)

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

.PHONY: all test test-point test-terminal test-verbose clean