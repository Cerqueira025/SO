# Variables

# C compiler - gcc
CC := gcc

# Compiler flags

# Notable flags
#	-Wdouble-promotion: "-Wdouble-promotion: This flag warns about implicit conversions that promote floating-point types to double. It helps catch potential precision loss issues."
#	-Werror=vla: "This flag treats variable-length array (VLA) usage as an error. VLAs are a C99 feature that allows arrays to have a length that is determined at runtime. Using VLAs can sometimes lead to unexpected runtime behavior or vulnerabilities."
#	-Wfatal-errors: "This flag causes the compiler to stop immediately after encountering the first error, rather than continuing with further compilation. It helps prevent the generation of potentially unreliable or incomplete executables."
CFLAGS := -std=c11 -Wall -Wextra -Wdouble-promotion -Werror=pedantic -Werror=vla -pedantic-errors -Wfatal-errors

# Is debug mode enabled? - 0 = no
DEBUG ?= 0


SRC_DIR := src
INC_DIR := include
OBJ_DIR := obj

CLIENT_FILES := client.c utils.c messages.c
SERVER_FILES := orchestrator.c utils.c messages.c

CLIENT_OBJ := $(addprefix $(OBJ_DIR)/, $(CLIENT_FILES:.c=.o))
SERVER_OBJ := $(addprefix $(OBJ_DIR)/, $(SERVER_FILES:.c=.o))


# Debug mode
# If make is ran as "make DEBUG=1", the program compiles without optimizations and allows gdb support
ifeq ($(DEBUG), 1)
        CFLAGS += -g -O0
else
        CFLAGS += -O3
endif


# Main target
all: client orchestrator

client: $(CLIENT_OBJ)
	@$(CC) $(CFLAGS) $^ -o $@;
	@echo " Successfully made client"

orchestrator: $(SERVER_OBJ)
	@$(CC) $(CFLAGS) $^ -o $@;
	@echo " Successfully made client"



# Object files compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# PASTA* IS TEMPORARY AND NEEDS TO BE REMOVED
clean:
	@rm -rf $(OBJ_DIR) client orchestrator fifo* TASK_* PASTA*
	@echo " Successfully cleaned"

# WHAT A BANGER OF A COMMAND. Cleans and re-compiles the program
rebuild: clean all

format:
	@clang-format --verbose -i $(SRC_DIR)/* $(INC_DIR)/*
	@echo " Successfully formatted"

check-format:
	@clang-format --dry-run --Werror $(SRC_DIR)/* $(INC_DIR)/*
	@echo " Successfully checked format"

lint:
	@clang-tidy --warnings-as-errors=* $(SRC_DIR)/* $(INC_DIR)/*
	@echo " Successfully linted"

# Delete object files if a command fails
.DELETE_ON_ERROR: