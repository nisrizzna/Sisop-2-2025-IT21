#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/types.h>
#include <pwd.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LOGFILE "debugmon.log"

void log_status(const char *process_name, const char *status) {
    FILE *log = fopen(LOGFILE, "a");
    if (!log) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(log, "[%02d:%02d:%d]-%02d:%02d:%02d_%s_%s\n",
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec,
            process_name, status);
    fclose(log);
}

uid_t get_uid_from_username(const char *username) {
    struct passwd *pw = getpwnam(username);
    return pw ? pw->pw_uid : -1;
}

void list_processes(uid_t uid) {
    DIR *proc = opendir("/proc");
    if (!proc) return;

    struct dirent *entry;
    while ((entry = readdir(proc)) != NULL) {
        if (!isdigit(*entry->d_name)) continue;

        char status_path[256], cmdline[256];
        snprintf(status_path, sizeof(status_path), "/proc/%s/status", entry->d_name);

        FILE *status = fopen(status_path, "r");
        if (!status) continue;

        uid_t proc_uid = -1;
        char name[256] = "Unknown";
        char line[256];
        while (fgets(line, sizeof(line), status)) {
            if (strncmp(line, "Uid:", 4) == 0)
                sscanf(line, "Uid:\t%u", &proc_uid);
            else if (strncmp(line, "Name:", 5) == 0)
                sscanf(line, "Name:\t%255s", name);
        }
        fclose(status);

        if (proc_uid == uid) {
            printf("PID: %s | CMD: %s\n", entry->d_name, name);
        }
    }

    closedir(proc);
}

void run_daemon(uid_t uid) {
    pid_t pid = fork();
    if (pid > 0) {
        printf("Running in daemon mode...\n");
        exit(0);
    } else if (pid == 0) {
        setsid();
        while (1) {
            DIR *proc = opendir("/proc");
            if (!proc) exit(1);

            struct dirent *entry;
            while ((entry = readdir(proc)) != NULL) {
                if (!isdigit(*entry->d_name)) continue;

                char status_path[256], name[256] = "Unknown";
                snprintf(status_path, sizeof(status_path), "/proc/%s/status", entry->d_name);

                FILE *status = fopen(status_path, "r");
                if (!status) continue;

                uid_t proc_uid = -1;
                char line[256];
                while (fgets(line, sizeof(line), status)) {
                    if (strncmp(line, "Uid:", 4) == 0)
                        sscanf(line, "Uid:\t%u", &proc_uid);
                    else if (strncmp(line, "Name:", 5) == 0)
                        sscanf(line, "Name:\t%255s", name);
                }
                fclose(status);

                if (proc_uid == uid)
                    log_status(name, "RUNNING");
            }

            closedir(proc);
            sleep(5);
        }
    }
}

void kill_user_processes(uid_t uid) {
    DIR *proc = opendir("/proc");
    if (!proc) return;

    struct dirent *entry;
    while ((entry = readdir(proc)) != NULL) {
        if (!isdigit(*entry->d_name)) continue;

        char status_path[256], name[256] = "Unknown";
        snprintf(status_path, sizeof(status_path), "/proc/%s/status", entry->d_name);

        FILE *status = fopen(status_path, "r");
        if (!status) continue;

        uid_t proc_uid = -1;
        char line[256];
        while (fgets(line, sizeof(line), status)) {
            if (strncmp(line, "Uid:", 4) == 0)
                sscanf(line, "Uid:\t%u", &proc_uid);
            else if (strncmp(line, "Name:", 5) == 0)
                sscanf(line, "Name:\t%255s", name);
        }
        fclose(status);

        if (proc_uid == uid) {
            kill(atoi(entry->d_name), SIGKILL);
            log_status(name, "FAILED");
        }
    }

    closedir(proc);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <command> <user>\n", argv[0]);
        return 1;
    }

    char *command = argv[1];
    char *username = argv[2];
    uid_t uid = get_uid_from_username(username);
    if (uid == -1) {
        printf("User not found.\n");
        return 1;
    }

    if (strcmp(command, "list") == 0) {
        list_processes(uid);
    } else if (strcmp(command, "daemon") == 0) {
        run_daemon(uid);
    } else if (strcmp(command, "stop") == 0) {
        system("pkill -f debugmon");
    } else if (strcmp(command, "fail") == 0) {
        kill_user_processes(uid);
    } else if (strcmp(command, "revert") == 0) {
        run_daemon(uid); // kembali ke mode daemon sebagai recovery
    } else {
        printf("Unknown command.\n");
    }

    return 0;
}
