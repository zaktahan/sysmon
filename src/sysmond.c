#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sysmon_ipc.h>

volatile sig_atomic_t stop = 0;

// Shared data
MemData mem_data = {0};
CpuData cpu_data = {0};
ProcData proc_data = {0};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void handler(int sig)
{
    (void)sig;
    stop = 1;
}

void printall(void)
{
    printf("\n===== MEMORY =====\n");
    printf("Total: %ld kB\n", mem_data.mem_total);
    printf("Available: %ld kB\n", mem_data.mem_avail);
    printf("Free: %ld kB\n", mem_data.mem_free);
    printf("Swap Total: %ld kB\n", mem_data.swap_total);
    printf("Swap Free: %ld kB\n", mem_data.swap_free);

    printf("\n===== CPU =====\n");
    printf("Load 1: %f\n", cpu_data.load1);
    printf("Load 5: %f\n", cpu_data.load5);
    printf("Load 15: %f\n", cpu_data.load15);

    printf("\n===== PROCESSES =====\n");
    printf("Total Processes: %d\n", proc_data.total_procs);
}

// Memory thread
void *mem_thread(void *arg)
{
    int fd = *(int *)arg;
    MemData temp_data;

    while (1)
    {
        if (read(fd, &temp_data, sizeof(temp_data))
            != sizeof(temp_data))
        {
            break;
        }

        pthread_mutex_lock(&mutex);

        mem_data = temp_data;
        printf("\nMEM updated\n");
        printall();

        pthread_mutex_unlock(&mutex);
    }

    kill(getpid(), SIGTERM);

    return NULL;
}

// CPU thread
void *cpu_thread(void *arg)
{
    int fd = *(int *)arg;
    CpuData temp_data;

    while (1)
    {
        if (read(fd, &temp_data, sizeof(temp_data))
            != sizeof(temp_data))
        {
            break;
        }

        pthread_mutex_lock(&mutex);

        cpu_data = temp_data;
        printf("\nCPU updated\n");
        printall();

        pthread_mutex_unlock(&mutex);
    }

    kill(getpid(), SIGTERM);

    return NULL;
}

// Process thread
void *proc_thread(void *arg)
{
    int fd = *(int *)arg;
    ProcData temp_data;

    while (1)
    {
        if (read(fd, &temp_data, sizeof(temp_data))
            != sizeof(temp_data))
        {
            break;
        }

        pthread_mutex_lock(&mutex);

        proc_data = temp_data;
        printf("\nPROC updated\n");
        printall();

        pthread_mutex_unlock(&mutex);
    }

    kill(getpid(), SIGTERM);

    return NULL;
}

int main(void)
{
    struct sigaction sa = {0};

    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1 ||
        sigaction(SIGTERM, &sa, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }

    int pipefd[2];

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return 1;
    }

    pid_t pid1 = fork();

    if (pid1 < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid1 == 0)
    {
        close(pipefd[0]);

        char fd_str[20];
        snprintf(fd_str, sizeof(fd_str), "%d", pipefd[1]);

        char *args[] = {"./bin/mem_sensor", fd_str, NULL};

        execve("./bin/mem_sensor", args, NULL);

        perror("execve");
        _exit(1);
    }

    close(pipefd[1]);

    if (mkfifo(CPU_FIFO_PATH, 0666) == -1 && errno != EEXIST)
    {
        perror("mkfifo cpu");
        return 1;
    }

    if (mkfifo(PROC_FIFO_PATH, 0666) == -1 && errno != EEXIST)
    {
        perror("mkfifo proc");
        return 1;
    }

    pid_t pid2 = fork();

    if (pid2 < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid2 == 0)
    {
        close(pipefd[0]);

        char *args[] = {"./bin/cpu_sensor", NULL};

        execve("./bin/cpu_sensor", args, NULL);

        perror("execve");
        _exit(1);
    }

    pid_t pid3 = fork();

    if (pid3 < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid3 == 0)
    {
        close(pipefd[0]);

        char *args[] = {"./bin/proc_sensor", NULL};

        execve("./bin/proc_sensor", args, NULL);

        perror("execve");
        _exit(1);
    }

    int cpu_fd = open(CPU_FIFO_PATH, O_RDONLY);

    if (cpu_fd < 0)
    {
        perror("open cpu FIFO");
        return 1;
    }

    int proc_fd = open(PROC_FIFO_PATH, O_RDONLY);

    if (proc_fd < 0)
    {
        perror("open proc FIFO");
        close(cpu_fd);
        return 1;
    }

    // Block signals before creating threads
    sigset_t mask;
    sigset_t oldmask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);

    pthread_t tid1;
    pthread_t tid2;
    pthread_t tid3;

    int created = 0;
    int ret = 0;

    if (pthread_sigmask(SIG_BLOCK, &mask, &oldmask) == 0)
    {
        // Create memory thread
        if (pthread_create(&tid1, NULL,
                           mem_thread, &pipefd[0]) == 0)
        {
            created++;
        }

        // Create CPU thread
        if (created == 1 &&
            pthread_create(&tid2, NULL,
                           cpu_thread, &cpu_fd) == 0)
        {
            created++;
        }

        // Create process thread
        if (created == 2 &&
            pthread_create(&tid3, NULL,
                           proc_thread, &proc_fd) == 0)
        {
            created++;
        }
    }

    if (created == 3)
    {
        // Wait for a termination signal
        while (!stop)
        {
            sigsuspend(&oldmask);
        }
    }
    else
    {
        fprintf(stderr, "Thread setup failed\n");
        ret = 1;
    }

    // Stop sensors
    kill(pid1, SIGTERM);
    kill(pid2, SIGTERM);
    kill(pid3, SIGTERM);

    // Wait for threads
    if (created >= 1)
        pthread_join(tid1, NULL);

    if (created >= 2)
        pthread_join(tid2, NULL);

    if (created >= 3)
        pthread_join(tid3, NULL);

    // Reap child processes
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    close(pipefd[0]);
    close(cpu_fd);
    close(proc_fd);

    unlink(CPU_FIFO_PATH);
    unlink(PROC_FIFO_PATH);

    pthread_mutex_destroy(&mutex);

    printf("\nSysmon stopped\n");

    return ret;
}
