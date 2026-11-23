#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sysmon.h>
volatile sig_atomic_t running = 1;

// Signal handler
void handler(int sig)
{
    running = 0;
}

int main(void)
{
  struct sigaction sa = {0};

    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, NULL);
    SystemState state = {0};
    pthread_t threads[4];

    // Initialize synchronization mechanisms
    pthread_mutex_init(&state.lock, NULL);
    pthread_cond_init(&state.alert_cond, NULL);

    // Create the four threads
    pthread_create(&threads[0], NULL, mem_sensor_thread, &state);
    pthread_create(&threads[1], NULL, cpu_sensor_thread, &state);
    pthread_create(&threads[2], NULL, proc_sensor_thread, &state);
    pthread_create(&threads[3], NULL, alert_thread, &state);

    // Main thread displays system information
    while (running)
    {
        pthread_mutex_lock(&state.lock);

        printf("\n===== MEMORY =====\n");
        printf("MemTotal: %ld kB\n", state.mem_total);
        printf("MemFree: %ld kB\n", state.mem_free);
        printf("MemAvailable: %ld kB\n", state.mem_avail);
        printf("SwapTotal: %ld kB\n", state.swap_total);
        printf("SwapFree: %ld kB\n", state.swap_free);

        printf("\n===== CPU =====\n");
        printf("Load 1 min: %f\n", state.load1);
        printf("Load 5 min: %f\n", state.load5);
        printf("Load 15 min: %f\n", state.load15);

        printf("\n===== PROCESSES =====\n");
        printf("Total processes: %d\n", state.total_procs);

        pthread_mutex_unlock(&state.lock);

        sleep(2);
    }

    /*
     * TODO: Add SIGINT handling for graceful shutdown.
     *
     * 1. Handle SIGINT (Ctrl+C).
     * 2. Set a shared running flag to 0.
     * 3. Make all worker threads exit their loops.
     * 4. Wake alert_thread if it is waiting.
     * 5. Join all worker threads.
     * 6. Destroy the mutex and condition variable.
     */

    // TODO: Execute this cleanup after implementing shutdown
    for (int i = 0; i < 4; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_cond_destroy(&state.alert_cond);
    pthread_mutex_destroy(&state.lock);

    return 0;
}
