CC = gcc
CFLAGS = -Wall -g

all: bin/mem_sensor bin/cpu_sensor bin/proc_sensor bin/sysmond

bin/mem_sensor: src/mem_sensor.c
	$(CC) $(CFLAGS) $< -o $@

bin/cpu_sensor: src/cpu_sensor.c
	$(CC) $(CFLAGS) $< -o $@

bin/proc_sensor: src/proc_sensor.c
	$(CC) $(CFLAGS) $< -o $@

bin/sysmond: src/sysmond.c
	$(CC) $(CFLAGS) $< -o $@

run: all
	./bin/sysmond

clean:
	rm -f bin/mem_sensor bin/cpu_sensor bin/proc_sensor bin/sysmond

kill:
	-killall sysmond

.PHONY: all clean run kill
