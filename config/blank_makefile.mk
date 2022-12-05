# Preexjsting variables brought in from master Makefile
SOURCE_DIR ?=
INCLUDE_DIR ?=
BUILD_DIR ?=
CFG_DIR ?=

TOOLCHAIN_32 ?=
TOOLCHAIN ?=

C_FLAGS ?=
CXX_FLAGS ?=
x64_FLAGS ?=
KERNEL_FLAGS ?=

LOCAL_DIR := $(notdir $(shell pwd))
LOCAL_INCLUDE := $(INCLUDE_DIR)/$(LOCAL_DIR)
LOCAL_SRC := $(SOURCE_DIR)/$(LOCAL_DIR)
LOCAL_BUILD := $(BUILD_DIR)/$(LOCAL_DIR)

LOCAL_FLAGS := -I$(LOCAL_INCLUDE)

LOCAL_OBJS := $(patsubst $(LOCAL_SRC)/%.cpp, $(LOCAL_BUILD)/%.o, $(wildcard $(LOCAL_SRC)/*.cpp))
LOCAL_OBJS += $(patsubst $(LOCAL_SRC)/%.c, $(LOCAL_BUILD)/%.o, $(wildcard $(LOCAL_SRC)/*.c))
LOCAL_OBJS += $(patsubst $(LOCAL_SRC)/%.s, $(LOCAL_BUILD)/%.o, $(wildcard $(LOCAL_SRC)/*.s))

SRC_OBJS += LOCAL_OBJS

.PHONY: all
all: $(LOCAL_OBJS)

$(LOCAL_BUILD):
	mkdir -p $@

$(LOCAL_OBJS): $(LOCAL_SRC)/%.cpp | $(LOCAL_BUILD)
	$(TOOLCHAIN)-g++ $(x64_FLAGS) $(CXX_FLAGS) $(KERNEL_FLAGS) $(LOCAL_FLAGS) -c $< -o $@

$(LOCAL_OBJS): $(LOCAL_SRC)/%.c | $(LOCAL_BUILD)
	$(TOOLCHAIN)-gcc $(x64_FLAGS) $(C_FLAGS) $(KERNEL_FLAGS) $(LOCAL_FLAGS) -c $< -o $@

$(LOCAL_OBJS): $(LOCAL_SRC)/%.s | $(LOCAL_BUILD)
	$(TOOLCHAIN)-gcc $(x64_FLAGS) $(C_FLAGS) $(KERNEL_FLAGS) -c $< -o $@