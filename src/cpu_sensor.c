#include <stdio.h>
#include <unistd.h>

int main(void)
{
while(1)
{
    FILE *fp = fopen("/proc/loadavg", "r");
    if (fp == NULL)
    {
        perror("fopen");
        return 1;
    }

    FILE *output = fopen("/tmp/sysmon/cpu.dat", "w");
    if (output == NULL)
    {
        perror("fopen");
        fclose(fp);
        return 1;
    }

    char line[256];
    float avg1, avg5, avg15;

    if (fgets(line, sizeof(line), fp) != NULL)
    {
        if (sscanf(line, "%f %f %f", &avg1, &avg5, &avg15) == 3)
        {
            fprintf(output, "%f %f %f\n", avg1, avg5, avg15);
        }
    }

    fclose(fp);
    fclose(output);
sleep(5);
}
    return 0;
}
