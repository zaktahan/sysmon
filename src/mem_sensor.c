#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
while(1)
{
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp == NULL)
    {
        perror("fopen");
        return 1;
    }

    FILE *output = fopen("/tmp/sysmon/mem.dat", "w");
    if (output == NULL)
    {
        perror("fopen");
        fclose(fp);
        return 1;
    }

    char line[256];

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        if (strstr(line, "MemTotal:") != NULL ||
            strstr(line, "MemAvailable:") != NULL ||
            strstr(line, "MemFree:") != NULL ||
            strstr(line, "SwapTotal:") != NULL ||
            strstr(line, "SwapFree:") != NULL)
        {
            fwrite(line, 1, strlen(line), output);
        }
    }

    fclose(fp);
    fclose(output);
sleep(5);
}
    return 0;
}
