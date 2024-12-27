# Compiler settings
CC = gcc
CFLAGS = -I./include
LDFLAGS =

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

# Find all source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
# Generate object file names
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
# Final executable name
TARGET = $(BIN_DIR)/opencc

# Default target
all: directories $(TARGET)

# Create necessary directories
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Link the final executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Clean and rebuild
rebuild: clean all

# Generate dependencies
depend: $(SRCS)
	$(CC) $(CFLAGS) -MM $^ > .depend

# Include dependencies if they exist
-include .depend

.PHONY: all clean rebuild directories depend