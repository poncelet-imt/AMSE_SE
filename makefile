CC= gcc
C_FLAG= -lm -lrt

BIN_DIR= ./bin
SRC_DIR= ./src

EXEC:= DCMotor \
	RegPID \
	ResetState \
	SetU \
	SetTv \
	Kinematics \
	Odometry \
	GO \
	TURN 

EXEC_MOTOR = DCMotor \
	RegPID \
	SetU \
	SetTv \
	ResetState


EXEC_MOTOR_PATH := $(EXEC_MOTOR:%=$(BIN_DIR)/%)
EXEC_PATH := $(EXEC:%=$(BIN_DIR)/%)


.PHONY: all motor clean

motor: $(EXEC_MOTOR_PATH)
	@echo "building motor loop"


all: $(EXEC_PATH)
	@echo "building all"

$(BIN_DIR)/%: $(SRC_DIR)/%.c
	$(CC) $^ $(C_FLAG) -o $@

clean:
	rm -rvf $(EXEC_PATH)
