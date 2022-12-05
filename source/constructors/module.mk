
MODULE := constructors
$(info Including $(MODULE))
$(info Build dir: $(BUILD_DIR)/$(MODULE))
$(info Source dir: $(SOURCE_DIR)/$(MODULE))

$(BUILD_DIR)/$(MODULE):
	mkdir -p $@

$(BUILD_DIR)/$(MODULE)/%.o: $(SOURCE_DIR)/$(MODULE)/%.s | $(BUILD_DIR)/$(MODULE)
	$(TOOLCHAIN)-gcc $(x64_FLAGS) $(C_FLAGS) -c $< -o $@