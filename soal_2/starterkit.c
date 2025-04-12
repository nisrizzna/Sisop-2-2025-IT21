#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define STARTER_KIT_DIR "starter_kit"
#define QUARANTINE_DIR "quarantine"
#define LOG_FILE "activity.log"

// Deklarasi fungsi
void log_activity(const char *message);
void show_usage();
char *get_current_timestamp();
void write_log(const char *msg);
void move_all_files(const char *src_dir, const char *dest_dir);
int delete_files(const char *dir);
int terminate_process(const char *procname);
void start_daemon(const char *exename, const char *procname);
int decode_files();
char *decode_base64(char *encoded);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        show_usage();
        return 1;
    }

    if (strcmp(argv[1], "--decrypt") == 0) {
        decode_files();
        log_activity("Decrypt process executed.");
    } else if (strcmp(argv[1], "--quarantine") == 0) {
        move_all_files(STARTER_KIT_DIR, QUARANTINE_DIR);
        log_activity("Quarantine process executed.");
    } else if (strcmp(argv[1], "--return") == 0) {
        move_all_files(QUARANTINE_DIR, STARTER_KIT_DIR);
        log_activity("Return process executed.");
    } else if (strcmp(argv[1], "--eradicate") == 0) {
        delete_files(STARTER_KIT_DIR);
        log_activity("Eradicate process executed.");
    } else if (strcmp(argv[1], "--shutdown") == 0) {
        terminate_process("starterkit");
        log_activity("Shutdown process executed.");
    } else {
        show_usage();
        return 1;
    }

    return 0;
}

// Implementasi fungsi
void log_activity(const char *message) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) return;
    char *timestamp = get_current_timestamp();
    fprintf(log, "[%s] - %s\n", timestamp, message);
    fclose(log);
    free(timestamp);
}

void show_usage() {
    printf("Usage: ./starterkit [--decrypt | --quarantine | --return | --eradicate | --shutdown]\n");
}

char *get_current_timestamp() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char *buf = malloc(20);
    strftime(buf, 20, "%d-%m-%Y %H:%M:%S", t);
    return buf;
}

void write_log(const char *msg) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        fprintf(log, "%s\n", msg);
        fclose(log);
    }
}

void move_all_files(const char *src_dir, const char *dest_dir) {
    DIR *dir = opendir(src_dir);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char src_path[512], dest_path[512];
            snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, entry->d_name);
            snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir, entry->d_name);
            rename(src_path, dest_path);

            char log_entry[1024];
            char *timestamp = get_current_timestamp();
            snprintf(log_entry, sizeof(log_entry), "[%s][%s] - %s - Successfully moved to %s directory.",
                     timestamp, timestamp, entry->d_name, dest_dir);
            write_log(log_entry);
            free(timestamp);
        }
    }
    closedir(dir);
}

int delete_files(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (!dir) return -1;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", dir_path, entry->d_name);
            remove(filepath);

            char log_entry[512];
            char *timestamp = get_current_timestamp();
            snprintf(log_entry, sizeof(log_entry), "[%s] - %s - File removed.", timestamp, entry->d_name);
            write_log(log_entry);
            free(timestamp);
        }
    }
    closedir(dir);
    return 0;
}

int terminate_process(const char *procname) {
    char command[256];
    snprintf(command, sizeof(command), "pkill -f %s", procname);
    int result = system(command);
    return result;
}

void start_daemon(const char *exename, const char *procname) {
    if (fork() == 0) {
        while (1) {
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "pgrep %s > /dev/null || ./%s &", procname, exename);
            system(cmd);
            sleep(5);
        }
    }
}

int decode_files() {
    DIR *dir = opendir(STARTER_KIT_DIR);
    if (!dir) return -1;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char *decoded = decode_base64(entry->d_name);
            if (decoded) {
                char old_path[512], new_path[512];
                snprintf(old_path, sizeof(old_path), "%s/%s", STARTER_KIT_DIR, entry->d_name);
                snprintf(new_path, sizeof(new_path), "%s/%s", STARTER_KIT_DIR, decoded);
                rename(old_path, new_path);

                char log_entry[512];
                char *timestamp = get_current_timestamp();
                snprintf(log_entry, sizeof(log_entry), "[%s] - %s -> %s", timestamp, entry->d_name, decoded);
                write_log(log_entry);
                free(timestamp);
                free(decoded);
            }
        }
    }
    closedir(dir);
    return 0;
}

char *decode_base64(char *encoded) {
    char command[512];
    snprintf(command, sizeof(command), "echo %s | base64 -D", encoded);

    FILE *fp = popen(command, "r");
    if (!fp) return NULL;

    char buffer[256];
    fgets(buffer, sizeof(buffer), fp);
    pclose(fp);

    char *result = malloc(strlen(buffer) + 1);
    strcpy(result, buffer);

    // Hapus karakter newline di akhir jika ada
    size_t len = strlen(result);
    if (len > 0 && result[len - 1] == '\n')
        result[len - 1] = '\0';

    return result;
}

