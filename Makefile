CC = gcc
CFLAGS = -g -Iinclude -Wall -Wextra -Wdouble-promotion -Werror=pedantic -Werror=vla -pedantic-errors -Wfatal-errors
LDFLAGS =

SRC_DIR := src
INC_DIR := include
OBJ_DIR := obj

CLIENT_FILES := client.c utils.c messages.c
SERVER_FILES := orchestrator.c utils.c messages.c

CLIENT_OBJ := $(addprefix $(OBJ_DIR)/, $(CLIENT_FILES:.c=.o))
SERVER_OBJ := $(addprefix $(OBJ_DIR)/, $(SERVER_FILES:.c=.o))

all: folders orchestrator client

orchestrator: bin/orchestrator

client: bin/client

rebuild: clean all

folders:
	@mkdir -p src include obj bin tmp

bin/orchestrator: $(SERVER_OBJ)
	$(CC) $(LDFLAGS) $^ -o $@
	@echo "\nSuccessfully made orchestrator\n"

bin/client: $(CLIENT_OBJ)
	$(CC) $(LDFLAGS) $^ -o $@
	@echo "\nSuccessfully made client\n"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	rm -f -r obj/* tmp/* bin/*
	@echo "\nSuccessfully cleaned\n"

format:
	@clang-format --verbose -i $(SRC_DIR)/* $(INC_DIR)/*
	@echo "\nSuccessfully formatted\n"

check-format:
	@clang-format --dry-run --Werror $(SRC_DIR)/* $(INC_DIR)/*
	@echo "\nSuccessfully checked format\n"

lint:
	@clang-tidy --warnings-as-errors=* $(SRC_DIR)/* $(INC_DIR)/*
	@echo "\nSuccessfully linted\n"

.DELETE_ON_ERROR:
