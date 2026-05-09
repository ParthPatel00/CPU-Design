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

clean:
	rm -f $(EMU_OBJS) $(ASM_OBJS) $(EMU_BIN) $(ASM_BIN)
	rm -f $(PROGRAMS)/*.bin

.PHONY: all programs clean
