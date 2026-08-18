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
    src/bitmask_imm.c \
    src/host_syscall.c \
    src/guest.c \
    src/elf_map.c \
    src/decode_insn.c \
    src/interp.c \
    src/syscall_bridge.c \
    src/runtime_syscall.c \
    src/translate.c \
    src/arm_exec.c \
    src/jit.c

OBJ = $(SRC:.c=.o)

TESTS = \
    tests/test_register \
    tests/test_assembler \
    tests/test_flags \
    tests/test_linux_abi \
    tests/test_decoder \
    tests/test_syscalls \
    tests/test_binary \
    tests/test_decode_insn \
    tests/test_interp \
    tests/test_elf_map \
    tests/test_translate

.PHONY: all test clean inventory inventory-binary oah

all: $(OBJ) oah

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

tests/%: tests/%.c $(OBJ)
	$(CC) $(CFLAGS) -o $@ $< $(OBJ)

oah: src/oah_main.o $(OBJ)
	$(CC) $(CFLAGS) -o $@ src/oah_main.o $(OBJ)

tests/hello.x86_64: tools/mk_hello_elf.py
	python3 tools/mk_hello_elf.py $@

test: $(TESTS) oah tests/hello.x86_64
	@fail=0; \
	for t in $(TESTS); do \
	    echo "== $$t"; \
	    ./$$t || fail=1; \
	done; \
	echo "== oah tests/hello.x86_64"; \
	./oah tests/hello.x86_64 > tests/hello.got || fail=1; \
	printf 'hello\n' | cmp -s - tests/hello.got || { echo "  FAIL hello output"; fail=1; }; \
	exit $$fail

inventory:
	python3 tools/extract_decomp_map.py ../rosetta_decomp.c -o inventory

inventory-binary:
	@test -f bin/rosetta || { echo "drop the ELF at reconstruct/bin/rosetta"; exit 1; }
	python3 tools/extract_from_binary.py bin/rosetta -o inventory/from_binary \
	    --inc src/host_syscall_table.inc

clean:
	rm -f $(OBJ) src/oah_main.o $(TESTS) oah tests/hello.x86_64 tests/hello.got
