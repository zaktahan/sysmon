CC = gcc
CFLAGS = -Wall -Wextra -pthread -Iinc

SRC = src/main.c src/mem_sensor.c src/cpu_sensor.c src/proc_sensor.c src/alert_thread.c

all: bin/sysmond

bin/sysmond: $(SRC) inc/sysmon.h
	mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o bin/sysmond

clean:
	rm -f bin/sysmond

.PHONY: all clean
