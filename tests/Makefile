# Compiler, Flags, Directory Name and Executable Name
CC= g++
CFLAGS= -std=c++11 -Wall -Wextra
ODIR= ./obj

# Test executable
TEST_ALL= runTests

all: $(ODIR) $(TEST_ALL)

$(ODIR):
	@mkdir -p $(ODIR)

# All tests in one executable (Catch requires single CATCH_CONFIG_MAIN)
$(TEST_ALL): testPoint.cpp catch.hpp
	$(CC) $(CFLAGS) testPoint.cpp -o $(TEST_ALL)

# Run all tests
test: all
	@echo "Running all tests..."
	./$(TEST_ALL)

# Delete objects and executables
clean:
	rm -rf $(ODIR) $(TEST_ALL)

.PHONY: all test clean