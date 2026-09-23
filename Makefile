TARGET      := ivan
TARGET_ARCH := x86_64

OUT     := out
BUILD   := build

# One runtime per platform, in the directory ivancc's -mtarget selects.
LINUX_DIR := $(BUILD)/lib/linux
EMU_DIR   := $(BUILD)/lib/ivanemu
RUNTIME   := $(LINUX_DIR)/crt0.o $(LINUX_DIR)/libc.o $(EMU_DIR)/crt0.o $(EMU_DIR)/libc.o
TARGET_SRC := libc/src/$(TARGET_ARCH)/target

CC      := gcc
CFLAGS  := -std=gnu99 -O2 -Iinclude -Iout -DTARGET_ARCH=$(TARGET_ARCH)
WARN    := -Wall -Wextra
LEX     := flex
YACC    := bison

MAIN_SRCS := src/cc.c src/ld.c src/as.c src/emu.c
ALL_SRCS  := $(shell find src -name '*.c')
LIB_SRCS  := $(filter-out $(MAIN_SRCS),$(ALL_SRCS))
LIB_OBJS  := $(patsubst src/%.c,$(OUT)/%.o,$(LIB_SRCS))
GEN_OBJS  := $(OUT)/lex.yy.o $(OUT)/parser.tab.o

CC_OBJS := $(OUT)/cc.o $(LIB_OBJS) $(GEN_OBJS)
# One translation unit now, so every tool that touches ELF also links its relocation pass.
ELF_OBJS := $(OUT)/obj/elf.o $(OUT)/util/file.o $(OUT)/util/str.o $(OUT)/arch/$(TARGET_ARCH)/rel.o

AS_OBJS := $(OUT)/as.o $(ELF_OBJS) \
	$(OUT)/arch/$(TARGET_ARCH)/txt.o $(OUT)/arch/$(TARGET_ARCH)/asm.o \
	$(OUT)/arch/$(TARGET_ARCH)/enc.o
EMU_OBJS := $(OUT)/emu.o $(ELF_OBJS) $(OUT)/arch/$(TARGET_ARCH)/emu.o
LD_OBJS := $(OUT)/ld.o $(ELF_OBJS)

TEST_TOOL  := tools/run_test
TEST_SRCS  := $(sort $(wildcard tests/test*.c))
TEST_NAMES := $(patsubst tests/%.c,%,$(TEST_SRCS))

CC_BIN := $(BUILD)/bin/$(TARGET)cc
AS_BIN := $(BUILD)/bin/$(TARGET)as
LD_BIN := $(BUILD)/bin/$(TARGET)ld
EMU_BIN := $(BUILD)/bin/$(TARGET)emu

# --- phony recipes ---
all: $(CC_BIN) $(AS_BIN) $(LD_BIN) $(EMU_BIN) $(RUNTIME)

clean:
	rm -rf $(BUILD) $(OUT)

tests: $(TEST_NAMES)

# --- tool recipes ---
$(CC_BIN): $(CC_OBJS) | $(BUILD)/bin
	$(CC) $(CFLAGS) $(WARN) $^ -o $@

$(AS_BIN): $(AS_OBJS) | $(BUILD)/bin
	$(CC) $(CFLAGS) $(WARN) $^ -o $@

$(LD_BIN): $(LD_OBJS) | $(BUILD)/bin
	$(CC) $(CFLAGS) $(WARN) $^ -o $@

$(EMU_BIN): $(EMU_OBJS) | $(BUILD)/bin
	$(CC) $(CFLAGS) $(WARN) $^ -o $@

# --- test recipes (one target per test, so `make test05_logical` works) ---
$(TEST_NAMES): %: tests/%.c $(TEST_TOOL) $(CC_BIN) $(RUNTIME)
	@$(TEST_TOOL) $<

# --- front-end generators ---
$(OUT)/parser.tab.c $(OUT)/parser.tab.h: src/ast/parser.y | $(OUT)
	$(YACC) -d -o $(OUT)/parser.tab.c $<

$(OUT)/lex.yy.c: src/ast/lexer.flex $(OUT)/parser.tab.h | $(OUT)
	$(LEX) -o $@ $<

$(OUT)/lex.yy.o: $(OUT)/lex.yy.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT)/parser.tab.o: $(OUT)/parser.tab.c
	$(CC) $(CFLAGS) -c $< -o $@

# --- objects (mirrors the src/ tree under out/) ---
$(OUT)/%.o: src/%.c $(OUT)/parser.tab.h | $(OUT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(WARN) -c $< -o $@

# --- runtime (libc/) recipes, one directory per platform ---
$(LINUX_DIR)/crt0.o: $(TARGET_SRC)/linux/crt0.s $(AS_BIN) | $(LINUX_DIR)
	$(AS_BIN) $< -o $@

$(LINUX_DIR)/libc.o: libc/src/libc.c $(CC_BIN) | $(LINUX_DIR)
	$(CC_BIN) -c $< -o $@

$(EMU_DIR)/crt0.o: $(TARGET_SRC)/ivanemu/crt0.s $(AS_BIN) | $(EMU_DIR)
	$(AS_BIN) $< -o $@

# The emulator's libc is the portable half bundled with its own I/O primitives.
$(OUT)/libc/ivanemu/core.o: libc/src/libc.c $(CC_BIN)
	@mkdir -p $(dir $@)
	$(CC_BIN) -c $< -o $@

$(OUT)/libc/ivanemu/sys.o: $(TARGET_SRC)/ivanemu/sys.s $(AS_BIN)
	@mkdir -p $(dir $@)
	$(AS_BIN) $< -o $@

$(EMU_DIR)/libc.o: $(OUT)/libc/ivanemu/core.o $(OUT)/libc/ivanemu/sys.o $(LD_BIN) | $(EMU_DIR)
	$(LD_BIN) -r $(OUT)/libc/ivanemu/core.o $(OUT)/libc/ivanemu/sys.o -o $@

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

.PHONY: all clean tests $(TEST_NAMES)
