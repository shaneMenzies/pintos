
KERNEL_SIZE_SCRIPT = $(BASE_DIR)/get_kernel_size.sh
BSS_SIZE_SCRIPT = $(BASE_DIR)/get_bss_size.sh

KERNEL_SIZE :=
BSS_SIZE :=

MODULE := bootstrap
BOOT_FLAGS = -I$(INCLUDE_DIR)/$(MODULE) -DKERNEL_SIZE=$(KERNEL_SIZE) -DBSS_SIZE=$(BSS_SIZE)
$(info Including $(MODULE))
$(info Build dir: $(BUILD_DIR)/$(MODULE))
$(info Source dir: $(SOURCE_DIR)/$(MODULE))

BOOT_OBJS := $(patsubst $(SOURCE_DIR)/$(MODULE)/%.cpp, $(BUILD_DIR)/$(MODULE)/%.o, $(wildcard $(SOURCE_DIR)/$(MODULE)/*.cpp))
BOOT_OBJS += $(patsubst $(SOURCE_DIR)/$(MODULE)/%.c, $(BUILD_DIR)/$(MODULE)/%.o, $(wildcard $(SOURCE_DIR)/$(MODULE)/*.c))
BOOT_OBJS += $(patsubst $(SOURCE_DIR)/$(MODULE)/%.s, $(BUILD_DIR)/$(MODULE)/%.o, $(wildcard $(SOURCE_DIR)/$(MODULE)/*.s))

BOOT_LINK_SCRIPT := $(CFG_DIR)/bootstrap.ld
BOOT_LD_FLAGS := -O2 -nostdlib -lgcc -g -Xlinker -Map=bootstrap.map -z max-page-size=0x1000
BOOTSTRAP_TARGET = pintos_boot.bin

TOTAL_TARGETS += $(BOOTSTRAP_TARGET)

.PHONY: GET_SIZES

GET_SIZES: | $(KERNEL_TARGET)
	$(eval KERNEL_SIZE := $(shell $(KERNEL_SIZE_SCRIPT)))
	$(eval BSS_SIZE := $(shell $(BSS_SIZE_SCRIPT)))

$(BOOTSTRAP_TARGET): $(BOOT_OBJS) GET_SIZES |
	$(info Kernel Size: $(KERNEL_SIZE))
	$(info BSS Size: $(BSS_SIZE))
	$(TOOLCHAIN)-g++ -T $(BOOT_LINK_SCRIPT) $(BOOT_LD_FLAGS) $(BOOT_OBJS) -o $(BOOTSTRAP_TARGET)

$(BUILD_DIR)/$(MODULE):
	mkdir -p $@

$(BUILD_DIR)/$(MODULE)/%.o: $(SOURCE_DIR)/$(MODULE)/%.cpp | $(BUILD_DIR)/$(MODULE) GET_SIZES
	$(TOOLCHAIN)-g++ $(x64_FLAGS) $(CXX_FLAGS) $(BOOT_FLAGS) -c $< -o $@

$(BUILD_DIR)/$(MODULE)/%.o: $(SOURCE_DIR)/$(MODULE)/%.c | $(BUILD_DIR)/$(MODULE) GET_SIZES
	$(TOOLCHAIN)-gcc $(x64_FLAGS) $(C_FLAGS) $(BOOT_FLAGS) -c $< -o $@

$(BUILD_DIR)/$(MODULE)/%.o: $(SOURCE_DIR)/$(MODULE)/%.s | $(BUILD_DIR)/$(MODULE) GET_SIZES
	$(TOOLCHAIN)-gcc $(x64_FLAGS) $(C_FLAGS) $(MODULE_FLAGS) -c $< -o $@