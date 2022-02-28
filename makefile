CC= gcc
C_FLAG= -lm -lrt

BIN_DIR= ./bin
SRC_DIR= ./src

EXEC:= DCMotor \
	RegPID \
	ResetState \
	SetU \
	SetTv \
	Kinematics


EXEC_PATH := $(EXEC:%=$(BIN_DIR)/%)


.PHONY: all

all: $(EXEC_PATH)
	@echo "building"

$(BIN_DIR)/%: $(SRC_DIR)/%.c
	$(CC) $^ $(C_FLAG) -o $@
