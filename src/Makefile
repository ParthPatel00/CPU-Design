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

all: $(EMU_BIN) $(ASM_BIN)

$(EMU_BIN): $(EMU_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(ASM_BIN): $(ASM_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(EMU_OBJS) $(ASM_OBJS) $(EMU_BIN) $(ASM_BIN)

.PHONY: all clean
