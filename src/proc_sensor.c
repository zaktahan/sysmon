
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sysmon_ipc.h>

int main(void)
{
    ProcData procdata = {0};

    int fd = open(PROC_FIFO_PATH, O_WRONLY);

    if (fd < 0)
    {
        perror("open FIFO");
        return 1;
    }

    while (1)
    {
        int counter = 0;

        DIR *dir = opendir("/proc");

        if (dir == NULL)
        {
            perror("opendir");
            close(fd);
            return 1;
        }

        struct dirent *entry;

        while ((entry = readdir(dir)) != NULL)
        {
            int len = strlen(entry->d_name);
            int flag = 1;

            for (int i = 0; i < len; i++)
            {
                if (!isdigit((unsigned char)entry->d_name[i]))
                {
                    flag = 0;
                    break;
                }
            }

            if (flag == 1)
            {
                counter++;
            }
        }

        closedir(dir);

        procdata.total_procs = counter;

        ssize_t written = write(fd, &procdata, sizeof(procdata));

        if (written < 0)
        {
            perror("write");
            close(fd);
            return 1;
        }

        if ((size_t)written != sizeof(procdata))
        {
            fprintf(stderr, "Incomplete write\n");
            close(fd);
            return 1;
        }

        sleep(5);
    }

    close(fd);
    return 0;
}
