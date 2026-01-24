#!/usr/bin/make -f

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The C and C++ compilers.
CC = /usr/bin/clang
CXX = /usr/bin/clang++

# compile C with /usr/bin/clang
C_DEFINES =
# C_DEFINES = -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES \
#     -DVK_USE_PLATFORM_WIN32_KHR -DWIN32_LEAN_AND_MEAN
C_INCLUDES =
C_FLAGS =

# The source directory.
SOURCE_DIR = src
# The build directory.
BUILD_DIR = build

# List of all .c source files.
SRC = $(wildcard $(SOURCE_DIR)/*.c)
# All .o files go to build dir.
OBJ = $(SRC:$(SOURCE_DIR)/%.c=$(BUILD_DIR)/%.o)
# Gcc/Clang will create these .d files containing dependencies.
DEP = $(OBJ:%.o=%.d)

.SILENT:

BIN = $(BUILD_DIR)/main

#=============================================================================

all: $(BIN)
.PHONY: all

clean:
	rm -rf $(BUILD_DIR)
.PHONY: clean

# Include all .d files
-include $(DEP)

# Build target for every single object file.
# The potential dependency on header files is covered
# by calling `-include $(DEP)`.
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	@echo -e "\e[32m""Building C object $@""\e[0m"
	mkdir -p $(@D)
	$(CC) $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MMD -MT $@ -MF $(@:%.o=%.d) -o $@ -c $<

$(BIN): $(OBJ)
	@echo -e "\e[1;32m""Linking C executable $@""\e[0m"
	mkdir -p $(@D)
	$(CC) $(OBJ) -o $@
