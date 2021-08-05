BASE_DIR := .
SOURCE_DIR := source
BOOTSTRAP_DIR := $(SOURCE_DIR)/bootstrap
INCLUDE_DIR := include
BUILD_DIR := build
CFG_DIR := config

TOOLCHAIN_32 = i686-elf
TOOLCHAIN := x86_64-elf

C_FLAGS := -I$(INCLUDE_DIR) -I$(INCLUDE_DIR)/acpi -ffreestanding -fno-isolate-erroneous-paths-attribute -nostdlib -z max-page-size=0x1000 -mno-red-zone -Ofast -Wall -Wextra -g
x64_FLAGS := -m64 -mcmodel=large
CXX_FLAGS := $(C_FLAGS) -fno-exceptions -fno-rtti
LD_FLAGS := -Ofast -nostdlib -lgcc -g -Xlinker -Map=kernel.map

BOOT_LD_FLAGS := -O2 -nostdlib -lgcc -g -Xlinker -Map=bootstrap.map

LINK_SCRIPT := $(CFG_DIR)/kernel.ld
BOOT_LINK_SCRIPT := $(CFG_DIR)/bootstrap.ld

BOOT_OBJS += $(patsubst $(BOOTSTRAP_DIR)/%.cpp, $(BUILD_DIR)/bootstrap/%.o, $(wildcard $(BOOTSTRAP_DIR)/*.cpp))
BOOT_OBJS += $(patsubst $(BOOTSTRAP_DIR)/%.s, $(BUILD_DIR)/bootstrap/%.o, $(wildcard $(BOOTSTRAP_DIR)/*.s))

SRC_OBJS +=$(patsubst $(SOURCE_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(wildcard $(SOURCE_DIR)/*.cpp))
SRC_OBJS +=$(patsubst $(SOURCE_DIR)/handlers/%.cpp, $(BUILD_DIR)/handlers/%.o, $(wildcard $(SOURCE_DIR)/handlers/*.cpp))
SRC_OBJS +=$(patsubst $(SOURCE_DIR)/%.s, $(BUILD_DIR)/%.o, $(wildcard $(SOURCE_DIR)/*.s))

CRT_BEGIN := $(shell $(TOOLCHAIN)-g++ $(x64_FLAGS) $(CXX_FLAGS) -print-file-name=crtbegin.o)
CRT_END := $(shell $(TOOLCHAIN)-g++ $(x64_FLAGS) $(CXX_FLAGS) -print-file-name=crtend.o)
CRT_I := $(BUILD_DIR)/constructors/crti.o
CRT_N := $(BUILD_DIR)/constructors/crtn.o

OBJ_LINK_LIST := $(CRT_I) $(CRT_BEGIN) $(SRC_OBJS) $(CRT_END) $(CRT_N) 

BOOTSTRAP_TARGET = pintos_boot.bin
KERNEL_TARGET = pintos_kernel.bin

all: $(BOOTSTRAP_TARGET) $(KERNEL_TARGET)

.PHONY: all clean

$(BOOTSTRAP_TARGET) : $(BOOT_OBJS) | $(BUILD_DIR)
	$(TOOLCHAIN_32)-g++ -T $(BOOT_LINK_SCRIPT) $(BOOT_LD_FLAGS) $(BOOT_OBJS) -o $(BOOTSTRAP_TARGET)

$(KERNEL_TARGET): $(OBJ_LINK_LIST) | $(BUILD_DIR)
	$(TOOLCHAIN)-g++ -T $(LINK_SCRIPT) $(LD_FLAGS) $(OBJ_LINK_LIST) -o $(KERNEL_TARGET)

$(BUILD_DIR): 
	mkdir -p $@
	mkdir -p $(BUILD_DIR)/bootstrap
	mkdir -p $(BUILD_DIR)/constructors
	mkdir -p $(BUILD_DIR)/handlers

$(CRT_I): $(SOURCE_DIR)/constructors/crti.s | $(BUILD_DIR)
	$(info $<)
	$(TOOLCHAIN)-gcc $(x64_FLAGS) $(C_FLAGS) -c $< -o $@

$(CRT_N): $(SOURCE_DIR)/constructors/crtn.s | $(BUILD_DIR)
	$(info $<)
	$(TOOLCHAIN)-gcc $(x64_FLAGS) $(C_FLAGS) -c $< -o $@

$(BUILD_DIR)/handlers/%.o: $(SOURCE_DIR)/handlers/%.cpp | $(BUILD_DIR)
	$(info $<)
	$(TOOLCHAIN)-g++ $(x64_FLAGS) $(CXX_FLAGS) -mgeneral-regs-only -c $< -o $@

$(BUILD_DIR)/bootstrap/%.o: $(SOURCE_DIR)/bootstrap/%.cpp | $(BUILD_DIR)
	$(info $<)
	$(TOOLCHAIN_32)-g++ $(CXX_FLAGS) -c $< -o $@

$(BUILD_DIR)/bootstrap/%.o: $(SOURCE_DIR)/bootstrap/%.s | $(BUILD_DIR)
	$(info $<)
	$(TOOLCHAIN_32)-gcc $(C_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(BUILD_DIR)
	$(info $<)
	$(TOOLCHAIN)-g++ $(x64_FLAGS) $(CXX_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.s | $(BUILD_DIR)
	$(info $<)
	$(TOOLCHAIN)-gcc $(x64_FLAGS) $(C_FLAGS) -c $< -o $@


clean:
	@$(RM) -rv $(BUILD_DIR)

