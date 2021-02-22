BASE_DIR := .
SOURCE_DIR := source
INCLUDE_DIR := include
BUILD_DIR := build
CFG_DIR := config

TOOLCHAIN = i686-elf
TOOLCHAIN_64 := x86_64-elf

C_FLAGS := -I$(INCLUDE_DIR) -Wall -Wextra -nostdlib -Og -ffreestanding -g
LD_FLAGS := -nostdlib -Map kernel.map -L. -lgcc -g

LINK_SCRIPT := $(CFG_DIR)/linker.ld

SRC_FILES := $(wildcard $(SOURCE_DIR)/*.c) $(wildcard $(SOURCE_DIR)/*.s)

OBJS = $(patsubst $(SOURCE_DIR)/%.c, $(BUILD_DIR)/%.o, $(wildcard $(SOURCE_DIR)/*.c)) 
OBJS +=$(patsubst $(SOURCE_DIR)/%.s, $(BUILD_DIR)/%.o, $(wildcard $(SOURCE_DIR)/*.s))

TARGET := pintos.bin

all: $(TARGET)

.PHONY: all clean

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(info $(OBJS))
	$(TOOLCHAIN)-ld -T $(LINK_SCRIPT) $(LD_FLAGS) $(OBJS) -o $(TARGET)


$(BUILD_DIR): 
	mkdir -p $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c | $(BUILD_DIR)
	$(TOOLCHAIN)-gcc $(C_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.s | $(BUILD_DIR)
	$(TOOLCHAIN)-as $< -o $@

clean:
	@$(RM) -rv $(BUILD_DIR)

