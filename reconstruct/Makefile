# Reconstruction from RosettaLinux decompilation
# Isolated from the sibling guessed rewrite in the repo root.

CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Werror -Wno-unused-parameter -O2 -g
CFLAGS  += -Iinclude -D_GNU_SOURCE

SRC = \
    src/register.c \
    src/result.c \
    src/assembler_buffer.c \
    src/assembler.c \
    src/flags.c \
    src/linux_abi.c \
    src/decoder.c \
    src/condition.c \
    src/icache.c \
    src/bitmask_imm.c

OBJ = $(SRC:.c=.o)

TESTS = \
    tests/test_register \
    tests/test_assembler \
    tests/test_flags \
    tests/test_linux_abi \
    tests/test_decoder

.PHONY: all test clean inventory

all: $(OBJ)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

tests/%: tests/%.c $(OBJ)
	$(CC) $(CFLAGS) -o $@ $< $(OBJ)

test: $(TESTS)
	@fail=0; \
	for t in $(TESTS); do \
	    echo "== $$t"; \
	    ./$$t || fail=1; \
	done; \
	exit $$fail

inventory:
	python3 tools/extract_decomp_map.py ../rosetta_decomp.c -o inventory

clean:
	rm -f $(OBJ) $(TESTS)
