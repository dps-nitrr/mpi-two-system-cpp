CXX      = mpicxx
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -DOMPI_SKIP_MPICXX
BIN      = bin
PROGS    = hello exchange pingpong

all: $(PROGS:%=$(BIN)/%)

$(BIN)/%: src/%.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BIN):
	mkdir -p $(BIN)

# 2 processes on this machine
run-local: all
	@for p in $(PROGS); do echo "### mpirun -np 2 $(BIN)/$$p"; mpirun -np 2 $(BIN)/$$p; echo; done

# 2 processes across the machines listed in ./hostfile
run-cluster: all
	@for p in $(PROGS); do echo "### mpirun -np 2 --hostfile hostfile $(BIN)/$$p"; mpirun -np 2 --hostfile hostfile $(BIN)/$$p; echo; done

clean:
	rm -rf $(BIN)

.PHONY: all run-local run-cluster clean
