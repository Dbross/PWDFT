# PWDFT Phase 4 Verification Suite Makefile

# Compiler settings
CXX = mpicxx
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -g
CXXFLAGS += -I../Nwpw/nwpwlib
CXXFLAGS += -I../Nwpw/nwpwlib/Parallel
CXXFLAGS += -I../Nwpw/nwpwlib/lattice
CXXFLAGS += -I../Nwpw/nwpwlib/C3dB
CXXFLAGS += -I../Nwpw/nwpwlib/Control
CXXFLAGS += -I../Nwpw/nwpwlib/ion
CXXFLAGS += -I../Nwpw/nwpwlib/device
CXXFLAGS += -I../Nwpw/nwpwlib/D3dB
CXXFLAGS += -I../Nwpw/nwpwlib/blas
CXXFLAGS += -I../Nwpw/nwpwlib/fftpack
CXXFLAGS += -I../Nwpw/nwpwlib/utilities
CXXFLAGS += -I../Nwpw/nwpwlib/parse
CXXFLAGS += -I../Nwpw/nwpwlib/psp_library
CXXFLAGS += -I../Nwpw

# Library flags
LDFLAGS = -L../build/nwpwlib -L../build/pspw -L../build/band
LIBS = -lnwpwlib -lpspw -lband -lm -lmpi

# Source files
SOURCES = verification_main.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = verification_suite

# Default target
all: $(TARGET)

# Build verification suite
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run verification tests
test: $(TARGET)
	@echo "Running PWDFT Phase 4 Verification Suite..."
	@echo "=========================================="
	mpirun -np 1 ./$(TARGET)

# Run with multiple MPI ranks
test-mpi: $(TARGET)
	@echo "Running PWDFT Phase 4 Verification Suite with MPI..."
	@echo "=========================================="
	mpirun -np 4 ./$(TARGET)

# Run specific test categories
test-physics: $(TARGET)
	@echo "Running Physics-Based Smoke Tests..."
	mpirun -np 1 ./$(TARGET) --physics-only

test-convergence: $(TARGET)
	@echo "Running Convergence Verification Tests..."
	mpirun -np 1 ./$(TARGET) --convergence-only

test-performance: $(TARGET)
	@echo "Running Performance Verification Tests..."
	mpirun -np 4 ./$(TARGET) --performance-only

test-stability: $(TARGET)
	@echo "Running Numerical Stability Tests..."
	mpirun -np 1 ./$(TARGET) --stability-only

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Clean everything
distclean: clean
	rm -f *.log *.out

# Install verification suite
install: $(TARGET)
	@mkdir -p ../bin
	cp $(TARGET) ../bin/
	@echo "Verification suite installed to ../bin/$(TARGET)"

# Help target
help:
	@echo "PWDFT Phase 4 Verification Suite Makefile"
	@echo "=========================================="
	@echo "Available targets:"
	@echo "  all              - Build verification suite"
	@echo "  test             - Run verification tests (1 rank)"
	@echo "  test-mpi         - Run verification tests (4 ranks)"
	@echo "  test-physics     - Run physics-based smoke tests"
	@echo "  test-convergence - Run convergence verification tests"
	@echo "  test-performance - Run performance verification tests"
	@echo "  test-stability   - Run numerical stability tests"
	@echo "  clean            - Remove build artifacts"
	@echo "  distclean        - Remove all generated files"
	@echo "  install          - Install verification suite"
	@echo "  help             - Show this help message"

# Dependencies
verification_main.o: verification_main.cpp verification_utils.hpp

.PHONY: all test test-mpi test-physics test-convergence test-performance test-stability clean distclean install help 