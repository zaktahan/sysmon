#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

int main(void)
{
while(1)
{
    int counter = 0;

    DIR *dir = opendir("/proc");

    if (dir == NULL)
    {
        perror("opendir");
        return 1;
    }
	FILE *output = fopen("/tmp/sysmon/proc.dat", "w");
if (output == NULL)
    {
     	perror("openfile");
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

    fprintf(output,"Number of processes: %d\n", counter);
	fclose(output);
sleep(5);
}
    return 0;
}
