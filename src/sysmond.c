#include <stdio.h>
#include <unistd.h>
#include<errno.h>
#include <sys/stat.h>
int main(void)
{
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
    return 0;
}
