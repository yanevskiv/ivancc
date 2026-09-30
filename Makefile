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

LINUX_DIR := $(BUILD)/lib/linux
EMU_DIR   := $(BUILD)/lib/ivanemu
RUNTIME   := $(LINUX_DIR)/crt0.o $(LINUX_DIR)/libc.o $(EMU_DIR)/crt0.o $(EMU_DIR)/libc.o
TARGET_SRC := src/libc/$(TARGET_ARCH)/target

CC      := gcc
CFLAGS  := -std=gnu99 -O2 -Ih -Iout -DTARGET_ARCH=$(TARGET_ARCH)
DEPFLAGS := -MMD -MP
WARN    := -Wall -Wextra
LDLIBS  := -lm
LEX     := flex
YACC    := bison

MAIN_SRCS := src/cc.c src/ld.c src/as.c src/emu.c
ALL_SRCS  := $(shell find src -path src/libc -prune -o -name '*.c' -print)
LIB_SRCS  := $(filter-out $(MAIN_SRCS),$(ALL_SRCS))
LIB_OBJS  := $(patsubst src/%.c,$(OUT)/%.o,$(LIB_SRCS))
GEN_OBJS  := $(OUT)/lex.yy.o $(OUT)/c.tab.o $(OUT)/pp.yy.o

CC_OBJS := $(OUT)/cc.o $(LIB_OBJS) $(GEN_OBJS)
ELF_OBJS := $(OUT)/util/object/elf.o $(OUT)/util/console/err.o $(OUT)/util/console/log.o $(OUT)/util/str.o

AS_OBJS := $(OUT)/as.o $(ELF_OBJS) $(OUT)/util/buf.o \
	$(OUT)/arch/$(TARGET_ARCH)/txt.o $(OUT)/arch/$(TARGET_ARCH)/asm.o \
	$(OUT)/arch/$(TARGET_ARCH)/enc.o
EMU_OBJS := $(OUT)/emu.o $(ELF_OBJS) $(OUT)/util/fp.o $(OUT)/arch/$(TARGET_ARCH)/load.o $(OUT)/arch/$(TARGET_ARCH)/emu.o
LD_OBJS := $(OUT)/ld.o $(ELF_OBJS) $(OUT)/arch/$(TARGET_ARCH)/link.o

SYS_HEADERS := $(patsubst libc/include/%,$(BUILD)/include/%,$(shell find libc/include -name '*.h' 2>/dev/null))

TEST_TOOL  := tests/run_test
CORE_NAMES := $(patsubst tests/%.c,%,$(sort $(wildcard tests/core/test*.c)))
BUG_NAMES  := $(patsubst tests/%.c,%,$(sort $(wildcard tests/bugs/bug*.c)))
EDGE_NAMES := $(patsubst tests/%.c,%,$(sort $(wildcard tests/edge/test*.c)))
LIBC_NAMES := $(patsubst tests/%.c,%,$(sort $(wildcard tests/libc/test*.c)))
ERROR_NAMES := $(patsubst tests/%.c,%,$(sort $(wildcard tests/errors/err*.c)))
TEST_NAMES := $(CORE_NAMES) $(BUG_NAMES) $(EDGE_NAMES) $(LIBC_NAMES) $(ERROR_NAMES)

CC_BIN := $(BUILD)/bin/$(TARGET)cc
AS_BIN := $(BUILD)/bin/$(TARGET)as
LD_BIN := $(BUILD)/bin/$(TARGET)ld
EMU_BIN := $(BUILD)/bin/$(TARGET)emu

TEST_DEPS := $(TEST_TOOL) $(CC_BIN) $(AS_BIN) $(LD_BIN) $(EMU_BIN) $(RUNTIME)

# --- phony recipes ---
all: $(CC_BIN) $(AS_BIN) $(LD_BIN) $(EMU_BIN) $(RUNTIME) $(SYS_HEADERS)

clean:
	rm -rf $(BUILD) $(OUT)

tests: test_core test_bugs test_edge test_libc test_errors

test_core: $(CORE_NAMES)

test_bugs: $(BUG_NAMES)

test_edge: $(EDGE_NAMES)

test_libc: $(LIBC_NAMES)

test_errors: $(ERROR_NAMES)

# --- tool recipes ---
$(CC_BIN): $(CC_OBJS) | $(BUILD)/bin
	$(CC) $(CFLAGS) $(WARN) $^ $(LDLIBS) -o $@

$(AS_BIN): $(AS_OBJS) | $(BUILD)/bin
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
$(LINUX_DIR)/crt0.o: $(TARGET_SRC)/linux/crt0.s $(AS_BIN) | $(LINUX_DIR)
	$(AS_BIN) $< -o $@

$(LINUX_DIR)/libc.o: src/libc/libc.c $(CC_BIN) | $(LINUX_DIR)
	$(CC_BIN) -c $< -o $@

$(EMU_DIR)/crt0.o: $(TARGET_SRC)/ivanemu/crt0.s $(AS_BIN) | $(EMU_DIR)
	$(AS_BIN) $< -o $@

$(OUT)/libc/ivanemu/libc.o: src/libc/libc.c $(CC_BIN)
	@mkdir -p $(dir $@)
	$(CC_BIN) -c $< -o $@

$(OUT)/libc/ivanemu/sys.o: $(TARGET_SRC)/ivanemu/sys.s $(AS_BIN)
	@mkdir -p $(dir $@)
	$(AS_BIN) $< -o $@

$(EMU_DIR)/libc.o: $(OUT)/libc/ivanemu/libc.o $(OUT)/libc/ivanemu/sys.o $(LD_BIN) | $(EMU_DIR)
	$(LD_BIN) -r $(OUT)/libc/ivanemu/libc.o $(OUT)/libc/ivanemu/sys.o -o $@

# --- system header recipes ---
$(BUILD)/include/%.h: libc/include/%.h
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

.PHONY: all clean tests test_core test_bugs test_edge test_libc test_errors $(TEST_NAMES)
