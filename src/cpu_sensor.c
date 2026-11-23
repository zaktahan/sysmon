#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sysmon_ipc.h>

int main(void)
{
    CpuData cpu_data = {0};
    char line[256];
    double avg1, avg5, avg15;

    int fd = open(CPU_FIFO_PATH, O_WRONLY);

    if (fd < 0)
    {
        perror("open FIFO");
        return 1;
    }

    while (1)
    {
        FILE *fp = fopen("/proc/loadavg", "r");

        if (fp == NULL)
        {
            perror("fopen");
            close(fd);
            return 1;
        }

        if (fgets(line, sizeof(line), fp) != NULL)
        {
            if (sscanf(line, "%lf %lf %lf",
                       &avg1, &avg5, &avg15) == 3)
            {
                cpu_data.load1 = avg1;
                cpu_data.load5 = avg5;
                cpu_data.load15 = avg15;
            }
            else
            {
                fprintf(stderr, "Failed to parse CPU load\n");
                fclose(fp);
                close(fd);
                return 1;
            }
        }
        else
        {
            fprintf(stderr, "Failed to read /proc/loadavg\n");
            fclose(fp);
            close(fd);
            return 1;
        }

        fclose(fp);

        // Send CpuData as raw binary data
        size_t total = 0;

        while (total < sizeof(cpu_data))
        {
            ssize_t written = write(
                fd,
                (const char *)&cpu_data + total,
                sizeof(cpu_data) - total
            );

            if (written < 0)
            {
                if (errno == EINTR)
                    continue;

                perror("write");
                close(fd);
                return 1;
            }

            if (written == 0)
            {
                fprintf(stderr, "write: no progress\n");
                close(fd);
                return 1;
            }

            total += written;
        }

        sleep(1);
    }

    close(fd);
    return 0;
}
