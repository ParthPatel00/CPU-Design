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

.PHONY: all programs test clean
