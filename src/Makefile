CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g

# Emulator
EMU_SRCS = main.c cpu.c memory.c alu.c bus.c control.c io.c
EMU_OBJS = $(EMU_SRCS:.c=.o)
EMU_BIN  = cpu

# Assembler
ASM_SRCS = asm_main.c assembler.c
ASM_OBJS = $(ASM_SRCS:.c=.o)
ASM_BIN  = asm

# Programs directory
PROGRAMS = ../programs

all: $(EMU_BIN) $(ASM_BIN)

$(EMU_BIN): $(EMU_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(ASM_BIN): $(ASM_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Assemble all programs into the programs/ directory
programs: $(ASM_BIN)
	@echo "[ASM] hello.asm"
	@./$(ASM_BIN) $(PROGRAMS)/hello.asm     $(PROGRAMS)/hello.bin
	@echo "[ASM] fibonacci.asm"
	@./$(ASM_BIN) $(PROGRAMS)/fibonacci.asm $(PROGRAMS)/fibonacci.bin
	@echo "[ASM] timer.asm"
	@./$(ASM_BIN) $(PROGRAMS)/timer.asm     $(PROGRAMS)/timer.bin
	@echo "[ASM] multiply.asm"
	@./$(ASM_BIN) $(PROGRAMS)/multiply.asm  $(PROGRAMS)/multiply.bin
	@echo "[ASM] primes.asm"
	@./$(ASM_BIN) $(PROGRAMS)/primes.asm    $(PROGRAMS)/primes.bin

# Automated test suite -- compares actual output against expected output
# Prints PASS or FAIL for each test. Exit code is non-zero if any test fails.
test: $(EMU_BIN) programs
	@echo ""
	@echo "========================================"
	@echo "  EduCore16 Automated Test Suite"
	@echo "========================================"
	@echo ""
	@PASS=0; FAIL=0; \
	\
	printf "  [TEST] hello        ... "; \
	ACTUAL=$$(./$(EMU_BIN) run $(PROGRAMS)/hello.bin 2>/dev/null | grep -v "^Loaded"); \
	EXPECT="Hello, World"; \
	if [ "$$ACTUAL" = "$$EXPECT" ]; then echo "PASS"; PASS=$$((PASS+1)); \
	else echo "FAIL"; echo "    expected: $$EXPECT"; echo "    got:      $$ACTUAL"; FAIL=$$((FAIL+1)); fi; \
	\
	printf "  [TEST] fibonacci    ... "; \
	ACTUAL=$$(./$(EMU_BIN) run $(PROGRAMS)/fibonacci.bin 2>/dev/null | grep -v "^Loaded" | tr -s ' ' | sed 's/^ //;s/ $$//' ); \
	EXPECT="0 1 1 2 3 5 8 13 21 34"; \
	if [ "$$ACTUAL" = "$$EXPECT" ]; then echo "PASS"; PASS=$$((PASS+1)); \
	else echo "FAIL"; echo "    expected: $$EXPECT"; echo "    got:      $$ACTUAL"; FAIL=$$((FAIL+1)); fi; \
	\
	printf "  [TEST] timer (last) ... "; \
	ACTUAL=$$(./$(EMU_BIN) run $(PROGRAMS)/timer.bin 2>/dev/null | grep -v "^Loaded" | tail -1); \
	EXPECT="[TIMER] 1"; \
	if [ "$$ACTUAL" = "$$EXPECT" ]; then echo "PASS"; PASS=$$((PASS+1)); \
	else echo "FAIL"; echo "    expected: $$EXPECT"; echo "    got:      $$ACTUAL"; FAIL=$$((FAIL+1)); fi; \
	\
	printf "  [TEST] timer (100)  ... "; \
	ACTUAL=$$(./$(EMU_BIN) run $(PROGRAMS)/timer.bin 2>/dev/null | grep -v "^Loaded" | head -1); \
	EXPECT="[TIMER] 100"; \
	if [ "$$ACTUAL" = "$$EXPECT" ]; then echo "PASS"; PASS=$$((PASS+1)); \
	else echo "FAIL"; echo "    expected: $$EXPECT"; echo "    got:      $$ACTUAL"; FAIL=$$((FAIL+1)); fi; \
	\
	printf "  [TEST] multiply     ... "; \
	ACTUAL=$$(./$(EMU_BIN) run $(PROGRAMS)/multiply.bin 2>/dev/null | grep -v "^Loaded" | tr -s ' ' | sed 's/^ //;s/ $$//'); \
	EXPECT="42"; \
	if [ "$$ACTUAL" = "$$EXPECT" ]; then echo "PASS"; PASS=$$((PASS+1)); \
	else echo "FAIL"; echo "    expected: $$EXPECT"; echo "    got:      $$ACTUAL"; FAIL=$$((FAIL+1)); fi; \
	\
	printf "  [TEST] fib dump     ... "; \
	ACTUAL=$$(./$(EMU_BIN) dump $(PROGRAMS)/fibonacci.bin 0x8000 0x8009 2>/dev/null | grep "^0x8000"); \
	EXPECT="0x8000  0x0000  0x0001  0x0001  0x0002  0x0003  0x0005  0x0008  0x000D  "; \
	if [ "$$ACTUAL" = "$$EXPECT" ]; then echo "PASS"; PASS=$$((PASS+1)); \
	else echo "FAIL"; echo "    expected: $$EXPECT"; echo "    got:      $$ACTUAL"; FAIL=$$((FAIL+1)); fi; \
	\
	printf "  [TEST] primes       ... "; \
	ACTUAL=$$(./$(EMU_BIN) run $(PROGRAMS)/primes.bin 2>/dev/null | grep -v "^Loaded" | tr -s ' ' | sed 's/^ //;s/ $$//'); \
	EXPECT="2 3 5 7 11 13 17 19 23 29"; \
	if [ "$$ACTUAL" = "$$EXPECT" ]; then echo "PASS"; PASS=$$((PASS+1)); \
	else echo "FAIL"; echo "    expected: $$EXPECT"; echo "    got:      $$ACTUAL"; FAIL=$$((FAIL+1)); fi; \
	\
	echo ""; \
	echo "  Results: $$PASS passed, $$FAIL failed"; \
	echo "========================================"; \
	echo ""; \
	test $$FAIL -eq 0

clean:
	rm -f $(EMU_OBJS) $(ASM_OBJS) $(EMU_BIN) $(ASM_BIN)
	rm -f $(PROGRAMS)/*.bin

# Part 1 Demo -- CPU Design
# Builds, assembles, and runs all programs showing actual output + memory dump
demo-part1: $(EMU_BIN) programs
	@echo ""
	@echo "========================================"
	@echo "  EduCore16 — Part 1: CPU Design Demo"
	@echo "========================================"
	@echo ""
	@echo "  [1] Hello, World"
	@echo "  -------------------"
	@./$(EMU_BIN) run $(PROGRAMS)/hello.bin 2>/dev/null
	@echo ""
	@echo "  [2] Fibonacci Sequence"
	@echo "  -------------------"
	@./$(EMU_BIN) run $(PROGRAMS)/fibonacci.bin 2>/dev/null
	@echo ""
	@echo "  [3] Timer Countdown (first 3 and last)"
	@echo "  -------------------"
	@./$(EMU_BIN) run $(PROGRAMS)/timer.bin 2>/dev/null | head -3
	@echo "  ..."
	@./$(EMU_BIN) run $(PROGRAMS)/timer.bin 2>/dev/null | tail -1
	@echo ""
	@echo "  [4] Recursive Multiply — multiply(6, 7)"
	@echo "  -------------------"
	@./$(EMU_BIN) run $(PROGRAMS)/multiply.bin 2>/dev/null
	@echo ""
	@echo "  [5] Prime Numbers"
	@echo "  -------------------"
	@./$(EMU_BIN) run $(PROGRAMS)/primes.bin 2>/dev/null
	@echo ""
	@echo "  [6] Fibonacci Memory Dump (RAM 0x8000-0x8009)"
	@echo "  -------------------"
	@./$(EMU_BIN) dump $(PROGRAMS)/fibonacci.bin 0x8000 0x8009 2>/dev/null
	@echo ""
	@echo "========================================"
	@echo "  Part 1 complete."
	@echo "========================================"
	@echo ""

# Part 2 Demo -- Program Layout and Execution
# Shows recursive multiply with full verbose trace and stack behavior
demo-part2: $(EMU_BIN) programs
	@echo ""
	@echo "========================================"
	@echo "  EduCore16 — Part 2: Program Layout and Execution Demo"
	@echo "========================================"
	@echo ""
	@echo "  [1] Run multiply(6, 7)"
	@echo "  -------------------"
	@./$(EMU_BIN) run $(PROGRAMS)/multiply.bin 2>/dev/null
	@echo ""
	@echo "  [2] Stack pointer trace (SP per cycle)"
	@echo "  -------------------"
	@./$(EMU_BIN) verbose $(PROGRAMS)/multiply.bin 2>/dev/null | grep "SP" | grep -v FLAGS
	@echo ""
	@echo "  [3] Final cycles — stack unwind and result"
	@echo "  -------------------"
	@./$(EMU_BIN) verbose $(PROGRAMS)/multiply.bin 2>/dev/null | tail -30
	@echo ""
	@echo "========================================"
	@echo "  Part 2 complete."
	@echo "  Stack: 0xFEFF -> 0xFEE9 (deepest) -> 0xFEFF (fully unwound)"
	@echo "  Result: R0 = 0x002A = 42"
	@echo "  Total cycles: 99"
	@echo "========================================"
	@echo ""

.PHONY: all programs test demo-part1 demo-part2 clean
