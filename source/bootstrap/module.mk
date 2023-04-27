
KERNEL_SIZE_SCRIPT = $(BASE_DIR)/get_kernel_size.sh
BSS_SIZE_SCRIPT = $(BASE_DIR)/get_bss_size.sh

KERNEL_SIZE :=
BSS_SIZE :=

BOOT_FLAGS = -DKERNEL_SIZE=$(KERNEL_SIZE) -DBSS_SIZE=$(BSS_SIZE)
BOOT_C_FLAGS = $(BOOT_FLAGS) -I$(INCLUDE_DIR)/$(MODULE) $(TOTAL_C_INCLUDE) -Wfatal-errors -ffreestanding -fno-isolate-erroneous-paths-attribute -nostdlib -z max-page-size=0x1000 -mpreferred-stack-boundary=4 -mno-red-zone $(TARGET_ARCH)  -Wall -Wno-array-bounds -Wno-virtual-move-assign
BOOT_CXX_FLAGS = $(BOOT_C_FLAGS) $(TOTAL_CXX_INCLUDE) -std=gnu++20 -Wno-pmf-conversions -fno-exceptions -fno-rtti -fno-use-cxa-atexit
MODULE := bootstrap
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
	$(TOOLCHAIN)-g++ $(x64_FLAGS) $(BOOT_CXX_FLAGS) -c $< -o $@

$(BUILD_DIR)/$(MODULE)/%.o: $(SOURCE_DIR)/$(MODULE)/%.c | $(BUILD_DIR)/$(MODULE) GET_SIZES
	$(TOOLCHAIN)-gcc  $(x64_FLAGS) $(BOOT_C_FLAGS) -c $< -o $@

$(BUILD_DIR)/$(MODULE)/%.o: $(SOURCE_DIR)/$(MODULE)/%.s | $(BUILD_DIR)/$(MODULE) GET_SIZES
	$(TOOLCHAIN)-gcc  $(x64_FLAGS) $(BOOT_FLAGS) -c $< -o $@
