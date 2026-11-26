
CC = gcc
CFLAGS = -Wall -Wextra -g -Iinc -pthread

SRC = src
BIN = bin

TARGETS = $(BIN)/sysmond $(BIN)/mem_sensor $(BIN)/cpu_sensor $(BIN)/proc_sensor

all: $(TARGETS)

$(BIN):
	mkdir -p $(BIN)

$(BIN)/sysmond: $(SRC)/sysmond.c inc/sysmon_ipc.h | $(BIN)
	$(CC) $(CFLAGS) -o $@ $<

$(BIN)/mem_sensor: $(SRC)/mem_sensor.c inc/sysmon_ipc.h | $(BIN)
	$(CC) $(CFLAGS) -o $@ $<

$(BIN)/cpu_sensor: $(SRC)/cpu_sensor.c inc/sysmon_ipc.h | $(BIN)
	$(CC) $(CFLAGS) -o $@ $<

$(BIN)/proc_sensor: $(SRC)/proc_sensor.c inc/sysmon_ipc.h | $(BIN)
	$(CC) $(CFLAGS) -o $@ $<

run: all
	./$(BIN)/sysmond

clean:
	rm -rf $(BIN)

.PHONY: all run clean
