#include <stdio.h>
#include <sysmon.h>

void *alert_thread(void *arg)
{
    SystemState *s = arg;

    while (1)
    {
        pthread_mutex_lock(&s->lock);

        while (s->alert == 0 && s->running)
        {
            pthread_cond_wait(&s->alert_cond, &s->lock);
        }

        if (!s->running)
        {
            pthread_mutex_unlock(&s->lock);
            break;
        }

        double load = s->load1;
        s->alert = 0;

        pthread_mutex_unlock(&s->lock);

        fprintf(stderr, "ALERT: high CPU load: %.2f\n", load);
    }

    return NULL;
}
