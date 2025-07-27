# FFT Debug Test Makefile
# Usage: make all, make test, make clean

CC = gcc
CFLAGS = -g -fsanitize=address -Wall -Wextra
LDFLAGS = -L../../build/nwpwlib -lnwpwlib -lm

# Test targets
TARGETS = run_complex_harnesses run_real_harnesses comprehensive_fft_test

.PHONY: all test clean

all: $(TARGETS)

# Complex FFT tests
run_complex_harnesses: test_all_complex.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

# Real FFT tests  
run_real_harnesses: test_all_real.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

# Comprehensive test
comprehensive_fft_test: comprehensive_fft_test.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

# Run all tests
test: all
	@echo "=== Running Complex FFT Tests ==="
	@./run_complex_harnesses
	@echo ""
	@echo "=== Running Real FFT Tests ==="
	@./run_real_harnesses
	@echo ""
	@echo "=== Running Comprehensive FFT Tests ==="
	@./comprehensive_fft_test
	@echo ""
	@echo "=== All tests completed ==="

# Clean up
clean:
	rm -f $(TARGETS) *.o

# Help
help:
	@echo "Available targets:"
	@echo "  all     - Build all test programs"
	@echo "  test    - Build and run all tests"
	@echo "  clean   - Remove built files"
	@echo "  help    - Show this help" 