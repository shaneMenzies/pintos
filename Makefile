BASE_DIR := .
SOURCE_DIR := source
INCLUDE_DIR := include
BUILD_DIR := build
CFG_DIR := config

TOOLCHAIN = i686-elf
TOOLCHAIN_64 := x86_64-elf

C_FLAGS := -I$(INCLUDE_DIR) -Wall -Wextra -nostdlib -Og -ffreestanding -g
CXX_FLAGS := $(C_FLAGS) -fno-exceptions -fno-rtti
LD_FLAGS := -nostdlib -Map kernel.map -L. -lgcc -g

LINK_SCRIPT := $(CFG_DIR)/linker.ld

SRC_FILES := $(wildcard $(SOURCE_DIR)/*.c) $(wildcard $(SOURCE_DIR)/*.s)

SRC_OBJS := $(patsubst $(SOURCE_DIR)/%.c, $(BUILD_DIR)/%.o, $(wildcard $(SOURCE_DIR)/*.c)) 
SRC_OBJS +=$(patsubst $(SOURCE_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(wildcard $(SOURCE_DIR)/*.cpp))
SRC_OBJS +=$(patsubst $(SOURCE_DIR)/%.s, $(BUILD_DIR)/%.o, $(wildcard $(SOURCE_DIR)/*.s))

CRT_BEGIN := $(shell $(TOOLCHAIN)-gcc $(CFLAGS) -print-file-name=crtbegin.o)
CRT_END := $(shell $(TOOLCHAIN)-gcc $(CFLAGS) -print-file-name=crtend.o)
CRT_I := $(BUILD_DIR)/constructors/crti.o
CRT_N := $(BUILD_DIR)/constructors/crtn.o

OBJ_LINK_LIST := $(CRT_I) $(CRT_BEGIN) $(SRC_OBJS) $(CRT_END) $(CRT_N)
BUILD_OBJS := $(CRT_I) $(SRC_OBJS) $(CTR_N)

TARGET := pintos.bin

all: $(TARGET)

.PHONY: all clean

$(TARGET): $(OBJ_LINK_LIST) | $(BUILD_DIR)
	$(info $(OBJ_LINK_LIST))
	$(TOOLCHAIN)-ld -T $(LINK_SCRIPT) $(LD_FLAGS) $(OBJ_LINK_LIST) -o $(TARGET)


$(BUILD_DIR): 
	mkdir -p $@
	mkdir -p $(BUILD_DIR)/constructors

$(CRT_I): $(SOURCE_DIR)/constructors/crti.s | $(BUILD_DIR)
	$(TOOLCHAIN)-gcc $(C_FLAGS) -c $< -o $@

$(CRT_N): $(SOURCE_DIR)/constructors/crtn.s | $(BUILD_DIR)
	$(TOOLCHAIN)-gcc $(C_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c | $(BUILD_DIR)
	$(TOOLCHAIN)-gcc $(C_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(BUILD_DIR)
	$(TOOLCHAIN)-g++ $(CXX_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.s | $(BUILD_DIR)
	$(TOOLCHAIN)-as $< -o $@

clean:
	@$(RM) -rv $(BUILD_DIR)

