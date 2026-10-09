# Makefile for the ivancc toolchain.
#
# Copyright (C) 2026 Ivan Janevski
#
# ivancc is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# ivancc is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with ivancc.  If not, see <https://www.gnu.org/licenses/>.

TARGET      := ivan
TARGET_ARCH := x86_64

OUT     := out
BUILD   := build

LINUX_DIR := $(BUILD)/lib/$(TARGET_ARCH)/linux
EMU_DIR   := $(BUILD)/lib/$(TARGET_ARCH)/ivanemu
RUNTIME   := $(LINUX_DIR)/crt0.o $(LINUX_DIR)/libc.a $(EMU_DIR)/crt0.o

CC      := gcc
CFLAGS  := -std=gnu99 -O2 -Ih -Iout -DTARGET_ARCH=$(TARGET_ARCH)
DEPFLAGS := -MMD -MP
WARN    := -Wall -Wextra
LDLIBS  := -lm
LEX     := flex
YACC    := bison

MAIN_SRCS := src/cc.c src/ld.c src/as.c src/ar.c src/emu.c
ALL_SRCS  := $(shell find src -path src/libc -prune -o -name '*.c' -print)
LIB_SRCS  := $(filter-out $(MAIN_SRCS),$(ALL_SRCS))
LIB_OBJS  := $(patsubst src/%.c,$(OUT)/%.o,$(LIB_SRCS))
GEN_OBJS  := $(OUT)/lex.yy.o $(OUT)/c.tab.o $(OUT)/pp.yy.o

CC_OBJS := $(OUT)/cc.o $(LIB_OBJS) $(GEN_OBJS)
ELF_OBJS := $(OUT)/util/object/elf.o $(OUT)/util/console/err.o $(OUT)/util/console/log.o $(OUT)/util/str.o

AS_OBJS := $(OUT)/as.o $(ELF_OBJS) $(OUT)/util/buf.o \
	$(OUT)/arch/$(TARGET_ARCH)/txt.o $(OUT)/arch/$(TARGET_ARCH)/asm.o \
	$(OUT)/arch/$(TARGET_ARCH)/enc.o
AR_OBJS := $(OUT)/ar.o $(ELF_OBJS) $(OUT)/util/object/lib.o
EMU_OBJS := $(OUT)/emu.o $(ELF_OBJS) $(OUT)/util/fp.o $(OUT)/arch/$(TARGET_ARCH)/cpu.o $(OUT)/util/object/load.o
LD_OBJS := $(OUT)/ld.o $(ELF_OBJS) $(OUT)/util/object/lib.o $(OUT)/util/object/link.o

LIBC_HEADERS := $(shell find h/libc -name '*.h')
LIBC_FLAGS   := -I h/libc
SYS_HEADERS  := $(patsubst h/libc/%,$(BUILD)/include/%,$(LIBC_HEADERS))
LIBC_SRCS    := $(wildcard src/libc/*.c src/libc/libc/*.c src/libc/libc/impl/*.c)
LIBC_OBJS    := $(patsubst src/libc/%.c,$(OUT)/libc/linux/%.o,$(LIBC_SRCS))

TEST_TOOL    := tests/run_test
SYNTAX_NAMES := $(patsubst tests/%.c,%,$(sort $(wildcard tests/syntax/syntax*.c)))
BUG_NAMES    := $(patsubst tests/%.c,%,$(sort $(wildcard tests/bugs/bug*.c)))
EDGE_NAMES   := $(patsubst tests/%.c,%,$(sort $(wildcard tests/edge/edge*.c)))
LIBC_NAMES   := $(patsubst tests/%.c,%,$(sort $(wildcard tests/libc/libc_test_*.c)))
ERROR_NAMES  := $(patsubst tests/%.c,%,$(sort $(wildcard tests/errors/err*.c tests/errors/fake_err*.c)))
TEST_NAMES   := $(SYNTAX_NAMES) $(BUG_NAMES) $(EDGE_NAMES) $(LIBC_NAMES) $(ERROR_NAMES)

CC_BIN := $(BUILD)/bin/$(TARGET)cc
AS_BIN := $(BUILD)/bin/$(TARGET)as
AR_BIN := $(BUILD)/bin/$(TARGET)ar
LD_BIN := $(BUILD)/bin/$(TARGET)ld
EMU_BIN := $(BUILD)/bin/$(TARGET)emu

TEST_DEPS := $(TEST_TOOL) $(CC_BIN) $(AS_BIN) $(AR_BIN) $(LD_BIN) $(EMU_BIN) $(RUNTIME) $(SYS_HEADERS)

# --- phony recipes ---
all: $(CC_BIN) $(AS_BIN) $(AR_BIN) $(LD_BIN) $(EMU_BIN) $(RUNTIME) $(SYS_HEADERS)

clean:
	rm -rf $(BUILD) $(OUT)

tests: test_syntax test_bugs test_edge test_libc test_errors

test_syntax: $(SYNTAX_NAMES)

test_bugs: $(BUG_NAMES)

test_edge: $(EDGE_NAMES)

test_libc: $(LIBC_NAMES)

test_errors: $(ERROR_NAMES)

# --- tool recipes ---
$(CC_BIN): $(CC_OBJS) | $(BUILD)/bin
	$(CC) $(CFLAGS) $(WARN) $^ $(LDLIBS) -o $@

$(AS_BIN): $(AS_OBJS) | $(BUILD)/bin
	$(CC) $(CFLAGS) $(WARN) $^ -o $@

$(AR_BIN): $(AR_OBJS) | $(BUILD)/bin
	$(CC) $(CFLAGS) $(WARN) $^ -o $@

$(LD_BIN): $(LD_OBJS) | $(BUILD)/bin
	$(CC) $(CFLAGS) $(WARN) $^ -o $@

$(EMU_BIN): $(EMU_OBJS) | $(BUILD)/bin
	$(CC) $(CFLAGS) $(WARN) $^ $(LDLIBS) -o $@

# --- test recipes ---
$(TEST_NAMES): %: tests/%.c $(TEST_DEPS)
	@$(TEST_TOOL) $<

# --- front-end generators ---
$(OUT)/c.tab.c $(OUT)/c.tab.h: src/lang/syntax/c.y | $(OUT)
	$(YACC) -d -o $(OUT)/c.tab.c $<

$(OUT)/lex.yy.c: src/lang/syntax/c.flex $(OUT)/c.tab.h | $(OUT)
	$(LEX) -o $@ $<

$(OUT)/pp.yy.c: src/lang/syntax/pp.flex | $(OUT)
	$(LEX) -o $@ $<

$(OUT)/lex.yy.o: $(OUT)/lex.yy.c
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(OUT)/c.tab.o: $(OUT)/c.tab.c
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(OUT)/pp.yy.o: $(OUT)/pp.yy.c
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

# --- objects ---
$(OUT)/%.o: src/%.c $(OUT)/c.tab.h | $(OUT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(WARN) $(DEPFLAGS) -c $< -o $@

# --- runtime recipes ---
$(LINUX_DIR)/crt0.o: src/libc/crt/crt.c $(CC_BIN) $(LIBC_HEADERS) | $(LINUX_DIR)
	$(CC_BIN) -mtarget=linux $(LIBC_FLAGS) -c $< -o $@

$(OUT)/libc/linux/%.o: src/libc/%.c $(CC_BIN) $(LIBC_HEADERS)
	@mkdir -p $(dir $@)
	$(CC_BIN) -mtarget=linux $(LIBC_FLAGS) -c $< -o $@

$(LINUX_DIR)/libc.a: $(LIBC_OBJS) $(AR_BIN) | $(LINUX_DIR)
	rm -f $@
	$(AR_BIN) rcs $@ $(LIBC_OBJS)

$(EMU_DIR)/crt0.o: src/libc/crt/crt.c $(CC_BIN) $(LIBC_HEADERS) | $(EMU_DIR)
	$(CC_BIN) -mtarget=ivanemu $(LIBC_FLAGS) -c $< -o $@

# --- system header recipes ---
$(BUILD)/include/%.h: h/libc/%.h
	@mkdir -p $(dir $@)
	cp $< $@

# --- build/ recipes ---
$(OUT):
	mkdir -p $(OUT)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/bin: | $(BUILD)
	mkdir -p $(BUILD)/bin

$(LINUX_DIR): | $(BUILD)
	mkdir -p $(LINUX_DIR)

$(EMU_DIR): | $(BUILD)
	mkdir -p $(EMU_DIR)

-include $(shell find $(OUT) -name '*.d' 2>/dev/null)

.PHONY: all clean tests test_syntax test_bugs test_edge test_libc test_errors $(TEST_NAMES)
