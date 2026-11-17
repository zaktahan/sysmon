#include <stdio.h>
#include <unistd.h>
#include <sysmon.h>

void *cpu_sensor_thread(void *arg)
{
    SystemState *s = arg;

    while (1)
    {
        pthread_mutex_lock(&s->lock);
        int running = s->running;
        pthread_mutex_unlock(&s->lock);

        if (!running)
            break;

        FILE *file = fopen("/proc/loadavg", "r");

        if (file == NULL)
        {
            perror("loadavg");
            sleep(2);
            continue;
        }

        double load1, load5, load15;

        if (fscanf(file, "%lf %lf %lf",
                   &load1, &load5, &load15) == 3)
        {
            pthread_mutex_lock(&s->lock);

            s->load1 = load1;
            s->load5 = load5;
            s->load15 = load15;

            if (load1 > 1.50)
            {
                s->alert = 1;
                pthread_cond_signal(&s->alert_cond);
            }

            pthread_mutex_unlock(&s->lock);
        }

        fclose(file);
        sleep(2);
    }

    return NULL;
}
