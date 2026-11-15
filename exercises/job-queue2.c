#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct job {
    int id;
    struct job* next;
};

struct job* job_queue = NULL;

pthread_mutex_t job_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

/* The work each thread performs */
void process_job(struct job* j)
{
    printf("Processing job %d\n", j->id);
}

/* Lecturer's function */
void* thread_function(void* arg)
{
    (void)arg;

    while (1) {
        struct job* next_job;

        pthread_mutex_lock(&job_queue_mutex);

        if (job_queue == NULL) {
            next_job = NULL;
        } else {
            next_job = job_queue;
            job_queue = job_queue->next;
        }

        pthread_mutex_unlock(&job_queue_mutex);

        if (next_job == NULL)
            break;

        process_job(next_job);
        free(next_job);
    }

    return NULL;
}

int main(void)
{
    pthread_t threads[2];

    // Step 1: Create jobs
    for (int i = 1; i <= 3; i++) {
        struct job* new_job = malloc(sizeof(struct job));

        if (new_job == NULL) {
            perror("malloc");
            return 1;
        }

        new_job->id = i;
        new_job->next = job_queue;
        job_queue = new_job;
    }

    // Step 2: Create threads
    for (int i = 0; i < 2; i++) {
        if (pthread_create(&threads[i], NULL,
                           thread_function, NULL) != 0) {
            fprintf(stderr, "pthread_create failed\n");
            return 1;
        }
    }

    // Step 3: Wait for threads
    for (int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&job_queue_mutex);

    return 0;
}

