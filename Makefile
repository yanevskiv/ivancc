TARGET      := ivan
TARGET_ARCH := x86_64

OUT     := out
BUILD   := build

LINUX_DIR := $(BUILD)/lib/linux
EMU_DIR   := $(BUILD)/lib/ivanemu
RUNTIME   := $(LINUX_DIR)/crt0.o $(LINUX_DIR)/libc.o $(EMU_DIR)/crt0.o $(EMU_DIR)/libc.o
TARGET_SRC := libc/src/$(TARGET_ARCH)/target

CC      := gcc
CFLAGS  := -std=gnu99 -O2 -Iinclude -Iout -DTARGET_ARCH=$(TARGET_ARCH)
DEPFLAGS := -MMD -MP
WARN    := -Wall -Wextra
LEX     := flex
YACC    := bison

MAIN_SRCS := src/cc.c src/ld.c src/as.c src/emu.c
ALL_SRCS  := $(shell find src -name '*.c')
LIB_SRCS  := $(filter-out $(MAIN_SRCS),$(ALL_SRCS))
LIB_OBJS  := $(patsubst src/%.c,$(OUT)/%.o,$(LIB_SRCS))
GEN_OBJS  := $(OUT)/lex.yy.o $(OUT)/c.tab.o

CC_OBJS := $(OUT)/cc.o $(LIB_OBJS) $(GEN_OBJS)
ELF_OBJS := $(OUT)/object/elf.o $(OUT)/util/file.o $(OUT)/util/str.o $(OUT)/arch/$(TARGET_ARCH)/rel.o

AS_OBJS := $(OUT)/as.o $(ELF_OBJS) \
	$(OUT)/arch/$(TARGET_ARCH)/txt.o $(OUT)/arch/$(TARGET_ARCH)/asm.o \
	$(OUT)/arch/$(TARGET_ARCH)/enc.o
EMU_OBJS := $(OUT)/emu.o $(ELF_OBJS) $(OUT)/arch/$(TARGET_ARCH)/emu.o
LD_OBJS := $(OUT)/ld.o $(ELF_OBJS)

TEST_TOOL  := tests/run_test
TEST_SRCS  := $(sort $(wildcard tests/syntax/test*.c))
TEST_NAMES := $(patsubst tests/syntax/%.c,%,$(TEST_SRCS))

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

# --- test recipes ---
$(TEST_NAMES): %: tests/syntax/%.c $(TEST_TOOL) $(CC_BIN) $(RUNTIME)
	@$(TEST_TOOL) $<

# --- front-end generators ---
$(OUT)/c.tab.c $(OUT)/c.tab.h: src/spec/c.y | $(OUT)
	$(YACC) -d -o $(OUT)/c.tab.c $<

$(OUT)/lex.yy.c: src/spec/c.flex $(OUT)/c.tab.h | $(OUT)
	$(LEX) -o $@ $<

$(OUT)/lex.yy.o: $(OUT)/lex.yy.c
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(OUT)/c.tab.o: $(OUT)/c.tab.c
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

# --- objects ---
$(OUT)/%.o: src/%.c $(OUT)/c.tab.h | $(OUT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(WARN) $(DEPFLAGS) -c $< -o $@

# --- runtime recipes ---
$(LINUX_DIR)/crt0.o: $(TARGET_SRC)/linux/crt0.s $(AS_BIN) | $(LINUX_DIR)
	$(AS_BIN) $< -o $@

$(LINUX_DIR)/libc.o: libc/src/libc.c $(CC_BIN) | $(LINUX_DIR)
	$(CC_BIN) -c $< -o $@

$(EMU_DIR)/crt0.o: $(TARGET_SRC)/ivanemu/crt0.s $(AS_BIN) | $(EMU_DIR)
	$(AS_BIN) $< -o $@

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

-include $(shell find $(OUT) -name '*.d' 2>/dev/null)

.PHONY: all clean tests $(TEST_NAMES)
