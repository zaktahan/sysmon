#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sysmon_ipc.h>

int main(int argc, char *argv[])
{
    char line[256];
    long value;

    int fd = STDOUT_FILENO;

    // Get target file descriptor
    if (argc > 1)
    {
        fd = atoi(argv[1]);
    }

    while (1)
    {
        MemData mem_data = {0};

        FILE *fp = fopen("/proc/meminfo", "r");

        if (fp == NULL)
        {
            perror("fopen");
            return 1;
        }

        // Read memory information
        while (fgets(line, sizeof(line), fp) != NULL)
        {
            if (strstr(line, "MemTotal:") != NULL)
            {
                if (sscanf(line, "MemTotal: %ld", &value) == 1)
                {
                    mem_data.mem_total = value;
                }
            }
            else if (strstr(line, "MemAvailable:") != NULL)
            {
                if (sscanf(line, "MemAvailable: %ld", &value) == 1)
                {
                    mem_data.mem_avail = value;
                }
            }
            else if (strstr(line, "MemFree:") != NULL)
            {
                if (sscanf(line, "MemFree: %ld", &value) == 1)
                {
                    mem_data.mem_free = value;
                }
            }
            else if (strstr(line, "SwapTotal:") != NULL)
            {
                if (sscanf(line, "SwapTotal: %ld", &value) == 1)
                {
                    mem_data.swap_total = value;
                }
            }
            else if (strstr(line, "SwapFree:") != NULL)
            {
                if (sscanf(line, "SwapFree: %ld", &value) == 1)
                {
                    mem_data.swap_free = value;
                }
            }
        }

        fclose(fp);

        // Send the entire binary structure
        size_t total = 0;

        while (total < sizeof(mem_data))
        {
            ssize_t written = write(
                fd,
                (const char *)&mem_data + total,
                sizeof(mem_data) - total
            );

            if (written < 0)
            {
                if (errno == EINTR)
                    continue;

                perror("write");
                return 1;
            }

            if (written == 0)
            {
                fprintf(stderr, "write: no progress\n");
                return 1;
            }

            total += written;
        }

        sleep(1);
    }

    return 0;
}
