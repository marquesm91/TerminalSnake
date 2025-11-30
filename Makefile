# ============================================================================
# Terminal Snake - Makefile
# ============================================================================
# Three versions available:
#   v1.0 (legacy)      - Original monolithic code in libs/
#   v2.0 (modular)     - Platform abstraction in libs/core/ and libs/platform/
#   v3.0 (clean_arch)  - Clean Architecture (DDD/Hexagonal) in src/
# ============================================================================

# Compiler and flags
CC= g++ 
CFLAGS= -std=c++11 -Wall -Wextra -Wpedantic
EFLAGS= -lncurses -lcurl -lssl -lcrypto
ODIR= ./obj
EDIR= ./bin

# Executables
EXEC_LEGACY= tsnake
EXEC_MODULAR= tsnake_modular
EXEC_CLEAN= tsnake_clean

# Source files
SRCS_LEGACY= main.cpp
SRCS_MODULAR= main_modular.cpp
SRCS_CLEAN= src/main.cpp

# Object files
OBJS_LEGACY= $(ODIR)/main.o
OBJS_MODULAR= $(ODIR)/main_modular.o
OBJS_CLEAN= $(ODIR)/src_main.o

# ============================================================================
# Main targets - easy version switching
# ============================================================================

# Default: build all versions
all: legacy modular clean_arch
	@echo ""
	@echo "✅ All versions built successfully!"
	@echo ""
	@echo "Run any version:"
	@echo "  ./bin/tsnake          # v1.0 Legacy"
	@echo "  ./bin/tsnake_modular  # v2.0 Modular"
	@echo "  ./bin/tsnake_clean    # v3.0 Clean Architecture"

# Build and run shortcuts
run: run-clean
run-legacy: legacy
	./$(EDIR)/$(EXEC_LEGACY)
run-modular: modular
	./$(EDIR)/$(EXEC_MODULAR)
run-clean: clean_arch
	./$(EDIR)/$(EXEC_CLEAN)

# Individual version builds
legacy: $(EDIR)/$(EXEC_LEGACY)
	@echo "Built: bin/tsnake (v1.0 Legacy)"

modular: $(EDIR)/$(EXEC_MODULAR)
	@echo "Built: bin/tsnake_modular (v2.0 Modular)"

clean_arch: $(EDIR)/$(EXEC_CLEAN)
	@echo "Built: bin/tsnake_clean (v3.0 Clean Architecture)"

# ============================================================================
# Build rules
# ============================================================================

$(ODIR):
	@mkdir -p $@

$(EDIR):
	@mkdir -p $@

# Legacy build
$(ODIR)/main.o: main.cpp | $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@ $(EFLAGS)

$(EDIR)/$(EXEC_LEGACY): $(OBJS_LEGACY) | $(EDIR)
	$(CC) $(CFLAGS) $^ -o $@ $(EFLAGS)

# Modular build
$(ODIR)/main_modular.o: main_modular.cpp | $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@ $(EFLAGS)

$(EDIR)/$(EXEC_MODULAR): $(OBJS_MODULAR) | $(EDIR)
	$(CC) $(CFLAGS) $^ -o $@ $(EFLAGS)

# Clean Architecture build
$(ODIR)/src_main.o: src/main.cpp | $(ODIR)
	$(CC) $(CFLAGS) -I./src -c $< -o $@ $(EFLAGS)

$(EDIR)/$(EXEC_CLEAN): $(OBJS_CLEAN) | $(EDIR)
	$(CC) $(CFLAGS) -I./src $^ -o $@ $(EFLAGS)

# ============================================================================
# Development tools
# ============================================================================

# Run tests
test:
	@cd tests && make test

# Code coverage (requires lcov)
coverage:
	@cd tests && make coverage

# Lines of code statistics
loc:
	@cd tests && make loc

# Full project statistics
stats:
	@cd tests && make stats

# Help
help:
	@echo "Terminal Snake - Build System"
	@echo ""
	@echo "Build targets:"
	@echo "  make              Build all versions"
	@echo "  make legacy       Build v1.0 (original)"
	@echo "  make modular      Build v2.0 (platform abstraction)"
	@echo "  make clean_arch   Build v3.0 (Clean Architecture)"
	@echo ""
	@echo "Run targets:"
	@echo "  make run          Run v3.0 (default)"
	@echo "  make run-legacy   Run v1.0"
	@echo "  make run-modular  Run v2.0"
	@echo "  make run-clean    Run v3.0"
	@echo ""
	@echo "Development:"
	@echo "  make test         Run all tests"
	@echo "  make coverage     Generate coverage report"
	@echo "  make loc          Count lines of code"
	@echo "  make stats        Show project statistics"
	@echo "  make clean        Remove build artifacts"

# Clean build artifacts
clean:
	rm -rf $(ODIR) $(EDIR)
	rm -rf *.settings

.PHONY: all run run-legacy run-modular run-clean legacy modular clean_arch test coverage loc stats help clean