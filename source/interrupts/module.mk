
MODULE := interrupts
MODULE_FLAGS := -I$(INCLUDE_DIR)/$(MODULE)
$(info Including $(MODULE))
$(info Build dir: $(BUILD_DIR)/$(MODULE))
$(info Source dir: $(SOURCE_DIR)/$(MODULE))

SRC_OBJS += $(patsubst $(SOURCE_DIR)/$(MODULE)/%.cpp, $(BUILD_DIR)/$(MODULE)/%.o, $(wildcard $(SOURCE_DIR)/$(MODULE)/*.cpp))
SRC_OBJS += $(patsubst $(SOURCE_DIR)/$(MODULE)/%.c, $(BUILD_DIR)/$(MODULE)/%.o, $(wildcard $(SOURCE_DIR)/$(MODULE)/*.c))
SRC_OBJS += $(patsubst $(SOURCE_DIR)/$(MODULE)/%.s, $(BUILD_DIR)/$(MODULE)/%.o, $(wildcard $(SOURCE_DIR)/$(MODULE)/*.s))

$(BUILD_DIR)/$(MODULE)/%.o: LOCAL_FLAGS := $(MODULE_FLAGS)

HANDLERS := int_handlers
HANDLER_FLAGS := -mgeneral-regs-only

$(BUILD_DIR)/$(MODULE):
	mkdir -p $@

$(BUILD_DIR)/$(MODULE)/$(HANDLERS).o: $(SOURCE_DIR)/$(MODULE)/$(HANDLERS).cpp | $(BUILD_DIR)/$(MODULE)
	$(TOOLCHAIN)-g++ $(x64_FLAGS) $(CXX_FLAGS) $(KERNEL_FLAGS) $(LOCAL_FLAGS) $(HANDLER_FLAGS) -c $< -o $@

$(BUILD_DIR)/$(MODULE)/%.o: $(SOURCE_DIR)/$(MODULE)/%.cpp | $(BUILD_DIR)/$(MODULE)
	$(TOOLCHAIN)-g++ $(x64_FLAGS) $(CXX_FLAGS) $(KERNEL_FLAGS) $(LOCAL_FLAGS) -c $< -o $@

$(BUILD_DIR)/$(MODULE)/%.o: $(SOURCE_DIR)/$(MODULE)/%.c | $(BUILD_DIR)/$(MODULE)
	$(TOOLCHAIN)-gcc $(x64_FLAGS) $(C_FLAGS) $(KERNEL_FLAGS) $(LOCAL_FLAGS) -c $< -o $@

$(BUILD_DIR)/$(MODULE)/%.o: $(SOURCE_DIR)/$(MODULE)/%.s | $(BUILD_DIR)/$(MODULE)
	$(TOOLCHAIN)-gcc $(x64_FLAGS) $(C_FLAGS) $(KERNEL_FLAGS) $(LOCAL_FLAGS) -c $< -o $@