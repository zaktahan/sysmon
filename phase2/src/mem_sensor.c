#include <stdio.h>
#include <unistd.h>
#include <sysmon.h>

void *mem_sensor_thread(void *arg)
{
    SystemState *s = arg;

    while (1)
    {
        pthread_mutex_lock(&s->lock);
        int running = s->running;
        pthread_mutex_unlock(&s->lock);

        if (!running)
            break;

        FILE *file = fopen("/proc/meminfo", "r");

        if (file == NULL)
        {
            perror("meminfo");
            sleep(2);
            continue;
        }

        long total = 0, free_mem = 0, avail = 0;
        long swap_total = 0, swap_free = 0;
        char line[256];

        while (fgets(line, sizeof(line), file))
        {
            sscanf(line, "MemTotal: %ld", &total);
            sscanf(line, "MemFree: %ld", &free_mem);
            sscanf(line, "MemAvailable: %ld", &avail);
            sscanf(line, "SwapTotal: %ld", &swap_total);
            sscanf(line, "SwapFree: %ld", &swap_free);
        }

        fclose(file);

        pthread_mutex_lock(&s->lock);

        s->mem_total = total;
        s->mem_free = free_mem;
        s->mem_avail = avail;
        s->swap_total = swap_total;
        s->swap_free = swap_free;

        pthread_mutex_unlock(&s->lock);

        sleep(2);
    }

    return NULL;
}
