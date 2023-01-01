export

BASE_DIR := $(shell pwd)
SOURCE_DIR := $(BASE_DIR)/source
INCLUDE_DIR := $(BASE_DIR)/include
BUILD_DIR := $(BASE_DIR)/build
CFG_DIR := $(BASE_DIR)/config
LIB_DIR := $(BASE_DIR)/lib
SUB_DIRS := $(notdir $(wildcard $(SOURCE_DIR)/*))

KERNEL_INCLUDE_DIR := ../include
KERNEL_LIB := kernel

C_INCLUDE_DIRS := $(INCLUDE_DIR) $(KERNEL_INCLUDE_DIR)
CPP_INCLUDE_DIRS :=
TOTAL_C_INCLUDE := $(foreach dir,$(C_INCLUDE_DIRS), -I$(dir))
TOTAL_CPP_INCLUDE := $(foreach dir,$(CPP_INCLUDE_DIRS), -I$(dir))


TOOLCHAIN := x86_64-pintos

DEBUG ?= 0
O_LEVEL := -Og
TARGET_ARCH :=
C_FLAGS := $(TOTAL_C_INCLUDE) -Wfatal-errors -fno-isolate-erroneous-paths-attribute -nostdlib -z max-page-size=0x1000 -mno-red-zone $(TARGET_ARCH) $(O_LEVEL) -Wall -Wextra -g
ifeq ($(DEBUG), 1)
	C_FLAGS += -DDEBUG=1
endif
CXX_FLAGS := $(C_FLAGS) $(TOTAL_CPP_INCLUDE) -std=gnu++20 -Wno-pmf-conversions -fno-exceptions -fno-rtti -fno-use-cxa-atexit
x64_FLAGS := -m64 -mcmodel=large

SRC_OBJS :=
CRT_BEGIN := $(shell $(TOOLCHAIN)-g++ $(x64_FLAGS) $(CXX_FLAGS) -print-file-name=crtbegin.o)
CRT_END := $(shell $(TOOLCHAIN)-g++ $(x64_FLAGS) $(CXX_FLAGS) -print-file-name=crtend.o)
CRT_I := $(BUILD_DIR)/constructors/crti.o
CRT_N := $(BUILD_DIR)/constructors/crtn.o

LIBC_TARGET := libpintos.a
TOTAL_TARGETS :=


include $(foreach dir,$(SUB_DIRS), $(wildcard $(SOURCE_DIR)/$(dir)/module.mk))

$(info Total objects: $(SRC_OBJS))
OBJ_LINK_LIST := $(CRT_I) $(CRT_BEGIN) $(SRC_OBJS) $(CRT_END) $(CRT_N)

TOTAL_TARGETS += $(LIBC_TARGET)

.PHONY: all clean

all: $(TOTAL_TARGETS)

$(LIBC_TARGET): $(OBJ_LINK_LIST) | $(BUILD_DIR)
	$(info "libc")
	$(TOOLCHAIN)-ar rcs $(LIBC_TARGET) $(OBJ_LINK_LIST)

$(BUILD_DIR):
	mkdir -p $@
	mkdir -p $(BUILD_DIR)/constructors

clean:
	@$(RM) -rv $(BUILD_DIR)
	@$(RM) $(TOTAL_TARGETS) $(LIBC_LIBRARY)
