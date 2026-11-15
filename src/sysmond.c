#include <stdio.h>
#include <unistd.h>
#include<errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
int main(void)
{
pid_t pid1 = fork();

if (pid1 < 0)
{
    perror("fork");
    return 1;
}

if (pid1 == 0)
{
    execl("./bin/mem_sensor", "mem_sensor", (char *)NULL);
    perror("execl");
    _exit(1);
}

pid_t pid2 = fork();

    if (pid2 == 0)
    {
        execl("./bin/cpu_sensor", "cpu_sensor", (char *)NULL);
        perror("execl");
        _exit(1);
    }
    if (pid2 < 0)
    {
        perror("fork");
        return 1;
    }

pid_t pid3 = fork();

    if (pid3 == 0)
    {
        execl("./bin/proc_sensor", "proc_sensor", (char *)NULL);
        perror("execl");
        _exit(1);
    }
    if (pid3 < 0)
    {
        perror("fork");
        return 1;
    }

if (mkdir("/tmp/sysmon", 0755) == -1 && errno != EEXIST)
{
    perror("mkdir");
    return 1;
}

while(1)
{

    FILE *sensor = fopen("/tmp/sysmon/proc.dat", "r");
    if (sensor == NULL)
    {
        perror("fopen");
        return 1;
    }

    FILE *cpu = fopen("/tmp/sysmon/cpu.dat", "r");
    if (cpu == NULL)
    {
        perror("fopen");
        fclose(sensor);
        return 1;
    }

    FILE *mem = fopen("/tmp/sysmon/mem.dat", "r");
    if (mem == NULL)
    {
        perror("fopen");
        fclose(cpu);
        fclose(sensor);
        return 1;
    }

    char line[256];

    printf("===== MEMORY =====\n");

    while (fgets(line, sizeof(line), mem) != NULL)
    {
        printf("%s", line);
    }

    printf("\n===== CPU =====\n");

    while (fgets(line, sizeof(line), cpu) != NULL)
    {
        printf("%s", line);
    }

    printf("\n===== PROCESSES =====\n");

    while (fgets(line, sizeof(line), sensor) != NULL)
    {
        printf("%s", line);
    }
	sleep(5);
    fclose(cpu);
    fclose(sensor);
    fclose(mem);
}
waitpid(pid1, NULL, 0);
waitpid(pid2, NULL, 0);
waitpid(pid3, NULL, 0);

    return 0;
}
