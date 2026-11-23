
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

void handler(int sig)
{
    (void)sig;
    stop = 1;
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

    MemData mem_data;
    CpuData cpu_data;
    ProcData proc_data;

    while (!stop)
    {
        if (read(pipefd[0], &mem_data, sizeof(mem_data)) != sizeof(mem_data))
        {
            if (!stop)
                fprintf(stderr, "Failed to read memory data\n");
            break;
        }

        if (stop)
            break;

        if (read(cpu_fd, &cpu_data, sizeof(cpu_data)) != sizeof(cpu_data))
        {
            if (!stop)
                fprintf(stderr, "Failed to read CPU data\n");
            break;
        }

        if (stop)
            break;

        if (read(proc_fd, &proc_data, sizeof(proc_data)) != sizeof(proc_data))
        {
            if (!stop)
                fprintf(stderr, "Failed to read process data\n");
            break;
        }

        if (stop)
            break;

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

        sleep(1);
    }

    kill(pid1, SIGTERM);
    kill(pid2, SIGTERM);
    kill(pid3, SIGTERM);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    close(pipefd[0]);
    close(cpu_fd);
    close(proc_fd);

    unlink(CPU_FIFO_PATH);
    unlink(PROC_FIFO_PATH);

    printf("\nSysmon stopped successfully\n");

    return 0;
}
