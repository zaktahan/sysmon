#include <stdio.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <sysmon.h>

void *proc_sensor_thread(void *arg)
{
    SystemState *s = arg;

    while (1)
    {
        pthread_mutex_lock(&s->lock);
        int running = s->running;
        pthread_mutex_unlock(&s->lock);

        if (!running)
            break;

        DIR *dir = opendir("/proc");

        if (dir == NULL)
        {
            perror("/proc");
            sleep(2);
            continue;
        }

        int count = 0;
        struct dirent *entry;

        while ((entry = readdir(dir)) != NULL)
        {
            int only_digits = 1;

            for (int i = 0; entry->d_name[i] != '\0'; i++)
            {
                if (!isdigit((unsigned char)entry->d_name[i]))
                {
                    only_digits = 0;
                    break;
                }
            }

            if (only_digits && entry->d_name[0] != '\0')
                count++;
        }

        closedir(dir);

        pthread_mutex_lock(&s->lock);
        s->total_procs = count;
        pthread_mutex_unlock(&s->lock);

        sleep(2);
    }

    return NULL;
}
