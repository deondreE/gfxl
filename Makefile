CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
LDFlAGS=
BUILD_DIR= build
SRC_DIR=src
TARGET=glfx
SRCS= $(wildcard $(SRC_DIR)/*.c)
OBJS=$(pathsubt $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all clean

all: $(BUILD_DIR) $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linking @"
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/.c
	@mkdir -p $(@D) 
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -p $@

$(BUILD_DIR):
	@mkdir -p  $(BUILD_DIR)

clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Clean complete."

.PHONY: clean all
