/*
 * daemonsync.c - Universal Daemon Manager
 * 
 * Copyright (C) 2024 D Brink <danie+daemonsync@brinkworx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>  // for PATH_MAX

#define MAX_CMD_LEN 1024
#define USER_RUN_DIR "userrun"
#define MAX_PATH_LEN 1024

char pid_file[256];
char cnf_file[256];
char log_file[256];
char program_name[128];
char run_dir[MAX_PATH_LEN];

void ensure_run_dir() {
    const char* home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "Error: HOME environment variable not set\n");
        exit(1);
    }
    
    snprintf(run_dir, sizeof(run_dir), "%s/%s", home, USER_RUN_DIR);
    
    struct stat st = {0};
    if (stat(run_dir, &st) == -1) {
        if (mkdir(run_dir, 0755) == -1) {
            perror("Error creating userrun directory");
            exit(1);
        }
    }
}

void init_file_paths(const char *arg0) {
    ensure_run_dir();
    
    char *base = basename(strdup(arg0));
    strncpy(program_name, base, sizeof(program_name) - 1);
    
    snprintf(pid_file, sizeof(pid_file), "%s/%s.pid", run_dir, program_name);
    snprintf(cnf_file, sizeof(cnf_file), "%s/%s.cnf", run_dir, program_name);
    snprintf(log_file, sizeof(log_file), "%s/%s.log", run_dir, program_name);
    free(base);
}

int read_pid() {
    FILE *fp = fopen(pid_file, "r");
    if (!fp) return -1;
    
    int pid;
    if (fscanf(fp, "%d", &pid) != 1) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return pid;
}

int is_process_running(int pid) {
    return kill(pid, 0) == 0;
}

void write_pid(int pid) {
    FILE *fp = fopen(pid_file, "w");
    if (fp) {
        fprintf(fp, "%d", pid);
        fclose(fp);
    }
}

void show_help() {
    printf("Usage:\n");
    printf("  status              - Check if daemon is running\n");
    printf("  stop                - Stop running daemon\n");
    printf("  run                 - Start daemon using configuration\n");
    printf("  setcnf <cmd> [args] - Set daemon configuration\n");
    printf("  getcnf              - Show current configuration\n\n");
    printf("Files:\n");
    printf("  PID file: %s\n", pid_file);
    printf("  Config file: %s\n", cnf_file);
    printf("  Log file: %s\n", log_file);
}

int check_status() {
    int pid = read_pid();
    if (pid > 0 && is_process_running(pid)) {
        printf("running\n");
        return 0;
    }
    printf("stopped\n");
    return 0;
}

int stop_daemon() {
    int pid = read_pid();
    if (pid <= 0 || !is_process_running(pid)) {
        printf("stopped\n");
        return 0;
    }

    // Try SIGINT first
    kill(pid, SIGINT);
    for (int i = 0; i < 10; i++) {
        sleep(1);
        if (!is_process_running(pid)) {
            printf("stopped\n");
            unlink(pid_file);
            return 0;
        }
    }

    // Force kill with SIGKILL
    kill(pid, SIGKILL);
    sleep(1);
    printf("terminated\n");
    unlink(pid_file);
    return 0;
}

int run_daemon() {
    // Check if already running
    int pid = read_pid();
    if (pid > 0 && is_process_running(pid)) {
        printf("running\n");
        return 1;
    }

    // Read configuration
    FILE *fp = fopen(cnf_file, "r");
    if (!fp) {
        printf("Error: No configuration file found\n");
        return 1;
    }

    char cmd[MAX_CMD_LEN];
    if (!fgets(cmd, sizeof(cmd), fp)) {
        fclose(fp);
        printf("Error: Empty configuration file\n");
        return 1;
    }
    fclose(fp);

    // Remove trailing newline
    cmd[strcspn(cmd, "\n")] = 0;

    // First fork
    pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid > 0) {
        // Parent process waits to check if daemon starts successfully
        for (int i = 0; i < 10; i++) {
            sleep(1);
            int daemon_pid = read_pid();
            if (daemon_pid > 0 && is_process_running(daemon_pid)) {
                printf("running\n");
                return 0;
            }
        }
        printf("stopped\n");
        return 0;
    }

    // Child process continues
    setsid();  // Create new session and process group

    // Second fork to fully daemonize
    pid = fork();
    if (pid < 0) {
        exit(1);
    }
    if (pid > 0) {
        exit(0);
    }

    // Save current working directory instead of changing to root
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        // If we can't get current directory, use home directory as fallback
        strncpy(cwd, getenv("HOME") ? getenv("HOME") : "/", sizeof(cwd));
    }
    chdir(cwd);

    // Close all open file descriptors
    for (int i = 0; i < sysconf(_SC_OPEN_MAX); i++) {
        close(i);
    }

    // Redirect standard file descriptors to log file
    int log_fd = open(log_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd < 0) {
        exit(1);
    }

    dup2(log_fd, STDOUT_FILENO);
    dup2(log_fd, STDERR_FILENO);
    close(log_fd);

    // Open /dev/null for stdin
    int null_fd = open("/dev/null", O_RDONLY);
    if (null_fd >= 0) {
        dup2(null_fd, STDIN_FILENO);
        close(null_fd);
    }

    // Write PID file
    pid = getpid();
    write_pid(pid);

    // Execute the command
    execl("/bin/sh", "sh", "-c", cmd, NULL);
    exit(1);  // Exit if execl fails
}

int set_config(int argc, char *argv[]) {
    // Check if daemon is running
    int pid = read_pid();
    if (pid > 0 && is_process_running(pid)) {
        printf("Error: Stop daemon before changing configuration\n");
        return 1;
    }

    FILE *fp = fopen(cnf_file, "w");
    if (!fp) {
        perror("Error opening config file");
        return 1;
    }

    // Combine arguments into command string
    for (int i = 2; i < argc; i++) {
        fprintf(fp, "%s%s", argv[i], (i < argc - 1) ? " " : "");
    }
    fprintf(fp, "\n");
    fclose(fp);
    printf("Configuration updated\n");
    return 0;
}

int get_config() {
    FILE *fp = fopen(cnf_file, "r");
    if (!fp) {
        printf("No configuration file found\n");
        return 1;
    }

    char line[MAX_CMD_LEN];
    while (fgets(line, sizeof(line), fp)) {
        printf("%s", line);
    }
    fclose(fp);
    return 0;
}

int main(int argc, char *argv[]) {
    init_file_paths(argv[0]);

    if (argc < 2) {
        show_help();
        return 0;
    }

    if (strcmp(argv[1], "help") == 0) {
        show_help();
        return 0;
    } else if (strcmp(argv[1], "status") == 0) {
        return check_status();
    } else if (strcmp(argv[1], "stop") == 0) {
        return stop_daemon();
    } else if (strcmp(argv[1], "run") == 0) {
        return run_daemon();
    } else if (strcmp(argv[1], "setcnf") == 0) {
        if (argc < 3) {
            printf("Error: setcnf requires a command\n");
            return 1;
        }
        return set_config(argc, argv);
    } else if (strcmp(argv[1], "getcnf") == 0) {
        return get_config();
    } else {
        show_help();
        return 1;
    }
}
