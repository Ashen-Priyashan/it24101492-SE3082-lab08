MPICC ?= mpicc
CFLAGS ?= -O2 -Wall -Wextra -std=c11
EXE = .exe
SHELL = cmd.exe

all: sum_mpi$(EXE) pi_monte_carlo$(EXE) pi_any_source$(EXE) pi_any_source_bsend$(EXE) message_mismatch$(EXE) message2_bsend$(EXE)

sum_mpi$(EXE): sum_mpi.c
	$(MPICC) $(CFLAGS) $< -o $@

pi_monte_carlo$(EXE): pi_monte_carlo.c
	$(MPICC) $(CFLAGS) $< -o $@

pi_any_source$(EXE): pi_any_source.c pi_monte_carlo.c
	$(MPICC) $(CFLAGS) pi_any_source.c -o $@

pi_any_source_bsend$(EXE): pi_any_source_bsend.c pi_monte_carlo.c
	$(MPICC) $(CFLAGS) pi_any_source_bsend.c -o $@

message_mismatch$(EXE): message_mismatch.c
	$(MPICC) $(CFLAGS) $< -o $@

message2_bsend$(EXE): message2_bsend.c
	$(MPICC) $(CFLAGS) $< -o $@

clean:
	-del /Q *.exe 2>NUL

.PHONY: all clean
