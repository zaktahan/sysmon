#include<sysmon.h>
#include<pthread.h>
#include<stdio.h>

void *alert_thread(void *arg)
{
    SystemState *state = arg;

    while (1)
    {
        pthread_mutex_lock(&state->lock);
	pthread_cond_wait(&state->alert_cond, &state->lock);
        pthread_mutex_unlock(&state->lock);
    fprintf(stderr, "WARNING: High CPU Load!\n");
}

    return NULL;
}
