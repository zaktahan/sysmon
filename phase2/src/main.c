#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sysmon.h>

SystemState state;

volatile sig_atomic_t stop_requested = 0;

void stop_handler(int sig)
{
    (void)sig;
    stop_requested = 1;
}

int main(void)
{
    pthread_t mem_thread;
    pthread_t cpu_thread;
    pthread_t proc_thread;
    pthread_t alert_tid;

    pthread_mutex_init(&state.lock, NULL);
    pthread_cond_init(&state.alert_cond, NULL);

    state.running = 1;

    signal(SIGINT, stop_handler);
    signal(SIGTERM, stop_handler);

    pthread_create(&mem_thread, NULL, mem_sensor_thread, &state);
    pthread_create(&cpu_thread, NULL, cpu_sensor_thread, &state);
    pthread_create(&proc_thread, NULL, proc_sensor_thread, &state);
    pthread_create(&alert_tid, NULL, alert_thread, &state);

    while (!stop_requested)
    {
        pthread_mutex_lock(&state.lock);

        printf("\033[2J\033[H");
        printf("========== SYSTEM MONITOR ==========\n");

        printf("\nMEMORY (kB):\n");
        printf("Total: %ld\n", state.mem_total);
        printf("Free: %ld\n", state.mem_free);
        printf("Available: %ld\n", state.mem_avail);
        printf("Swap total: %ld\n", state.swap_total);
        printf("Swap free: %ld\n", state.swap_free);

        printf("\nCPU LOAD:\n");
        printf("1 min: %f\n", state.load1);
        printf("5 min: %f\n", state.load5);
        printf("15 min: %f\n", state.load15);

        printf("\nPROCESSES: %d\n", state.total_procs);

        fflush(stdout);

        pthread_mutex_unlock(&state.lock);

        sleep(2);
    }

    pthread_mutex_lock(&state.lock);

    state.running = 0;
    pthread_cond_broadcast(&state.alert_cond);

    pthread_mutex_unlock(&state.lock);

    pthread_join(mem_thread, NULL);
    pthread_join(cpu_thread, NULL);
    pthread_join(proc_thread, NULL);
    pthread_join(alert_tid, NULL);

    pthread_cond_destroy(&state.alert_cond);
    pthread_mutex_destroy(&state.lock);

    printf("\nStopped.\n");

    return 0;
}
