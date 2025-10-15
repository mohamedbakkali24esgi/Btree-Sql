# Simple Makefile for class_db
# Usage examples:
#   make                # build the main binary
#   make SAN=1          # build with sanitizers
#   make test           # build and run tests
#   make clean          # remove build artifacts

CC      := gcc
CSTD    := -std=c17
WARN    := -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wcast-align -Wstrict-prototypes
OPT     := -O0
DBG     := -g3
DEFS    := -D_GNU_SOURCE
INCS    := -Iinclude
SRCS    := src/main.c src/db.c src/btree.c src/parser.c
OBJS    := $(SRCS:.c=.o)
TARGET  := class_db

# Optional sanitizers (set SAN=1 to enable)
ifeq ($(SAN),1)
SANFLAGS := -fsanitize=address,undefined -fno-omit-frame-pointer
LDFLAGS  := $(SANFLAGS)
CFLAGS   := $(CSTD) $(WARN) $(OPT) $(DBG) $(DEFS) $(INCS) $(SANFLAGS)
else
LDFLAGS  :=
CFLAGS   := $(CSTD) $(WARN) $(OPT) $(DBG) $(DEFS) $(INCS)
endif

.PHONY: all clean test format

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) tests/test_basic

# --- Tests ---
# Minimal assertion-based test
tests/test_basic: tests/test_basic.c src/db.c src/btree.c src/parser.c
	$(CC) $(CFLAGS) $(INCS) $^ -o $@ $(LDFLAGS)

test: tests/test_basic
	./tests/test_basic

# --- Formatting (optional if clang-format is installed) ---
format:
	@command -v clang-format >/dev/null 2>&1 && \
	  clang-format -i $(SRCS) include/*.h tests/*.c || \
	  echo "clang-format not found; skipping."
