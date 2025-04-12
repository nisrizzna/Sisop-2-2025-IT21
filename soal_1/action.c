#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <errno.h>

void download_and_unzip() {
 DIR* dir = opendir("Clues");
    if (dir) {
        closedir(dir);
        printf("Clues folder already exists. Skipping download.\n");
        return;
    }    remove("Clues.zip");

    printf("Downloading Clues.zip...\n");
    
    pid_t pid_download = fork();
    if (pid_download == 0) {
        execlp("wget", "wget", "https://drive.usercontent.google.com/u/0/uc?id=1xFn1OBJUuSdnApDseEczKhtNzyGekauK&export=download", "-O", "Clues.zip", NULL);
        perror("Failed to running wget");
        exit(1);
    } else if (pid_download > 0) {
        int status;
        waitpid(pid_download, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Downloading complete!\n");
            
            printf("Extracting Clues.zip...\n");
            pid_t pid_unzip = fork();
            if (pid_unzip == 0) {
                execlp("unzip", "unzip", "Clues.zip", NULL);
                perror("Failed to running unzip");
                exit(1);
            } else if (pid_unzip > 0) {
                waitpid(pid_unzip, &status, 0);
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                    printf("Extracting complete!\n");
                    remove("Clues.zip");
                } else {
                    printf("Failed to extracting file.\n");
                }
            } else {
                perror("Failed to running unzip process");
            }
        } else {
            printf("Failed to download! Check your internet connection and make sure you use the right link.\n");
        }
    } else {
        perror("Failed to starting download");
    }
}


void filter_files() {
    if (mkdir("Filtered", 0755) == -1 && errno != EEXIST) {
        perror("Failed to create Filtered directory");
        return;
    }

    printf("Starting filtering process...\n");
    int moved = 0, deleted = 0;

    const char *subdirs[] = {"ClueA", "ClueB", "ClueC", "ClueD", NULL};

    for (int i = 0; subdirs[i] != NULL; i++) {
        char dirpath[512];
        snprintf(dirpath, sizeof(dirpath), "Clues/%s", subdirs[i]);

        DIR *dir = opendir(dirpath);
        if (!dir) {
            perror("Failed to open clue directory");
            continue;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char *ext = strrchr(entry->d_name, '.');
            if (ext && strcmp(ext, ".txt") == 0) {

                int name_len = ext - entry->d_name;
                
                if (name_len == 1 && isalnum(entry->d_name[0])) {
                    char src_path[512], dest_path[512];
                    snprintf(src_path, sizeof(src_path), "Clues/%s/%s", subdirs[i], entry->d_name);
                    snprintf(dest_path, sizeof(dest_path), "Filtered/%s", entry->d_name);

                    if (rename(src_path, dest_path) == 0) {
                        printf("Moved %s/%s to Filtered\n", subdirs[i], entry->d_name);
                        moved++;
                    } else {
                        perror("Failed to move file");
                    }
                } else {
                    
                    char filepath[512];
                    snprintf(filepath, sizeof(filepath), "Clues/%s/%s", subdirs[i], entry->d_name);
                    if (remove(filepath) == 0) {
                        printf("Deleted %s/%s\n", subdirs[i], entry->d_name);
                        deleted++;
                    } else {
                        perror("Failed to delete file");
                    }
                }
            }
        }
        closedir(dir);
    }
    printf("Filtering complete. Moved %d files, deleted %d files.\n", moved, deleted);
}

void combine_files() {
    printf("Combining files in alternating order...\n");
    FILE *combined = fopen("Combined.txt", "w");
    if (!combined) {
        perror("Failed to create Combined.txt");
        return;
    }

    int processed = 0;
    
    for (int i = 0; i < 6; i++) {
        
        char num_file[20];
        sprintf(num_file, "Filtered/%d.txt", i+1);
        
        FILE *num_fp = fopen(num_file, "r");
        if (num_fp) {
            int c;
            while ((c = fgetc(num_fp)) != EOF) {
                fputc(c, combined);
            }
            fclose(num_fp);
            remove(num_file);
            processed++;
            printf("Added %s to Combined.txt\n", num_file);
        }

        char letter_file[20];
        sprintf(letter_file, "Filtered/%c.txt", 'a' + i);
        
        FILE *letter_fp = fopen(letter_file, "r");
        if (letter_fp) {
            int c;
            while ((c = fgetc(letter_fp)) != EOF) {
                fputc(c, combined);
            }
            fclose(letter_fp);
            remove(letter_file);
            processed++;
            printf("Added %s to Combined.txt\n", letter_file);
        }
    }

    fclose(combined);
    printf("Combining complete. %d files processed. Result in Combined.txt\n", processed);
}

void decode_rot13() {
    printf("Decoding Combined.txt using ROT13...\n");
    
    FILE *in = fopen("Combined.txt", "r");
    FILE *out = fopen("Decoded.txt", "w");
    
    if (!in || !out) {
        perror("Failed to open files");
        if (in) fclose(in);
        if (out) fclose(out);
        return;
    }

    int c;
    while ((c = fgetc(in)) != EOF) {
        if (isalpha(c)) {
            char base = islower(c) ? 'a' : 'A';
            c = ((c - base + 13) % 26) + base;
        }
        fputc(c, out);
    }
    
    fclose(in);
    fclose(out);
    printf("Decoding complete. Password saved to Decoded.txt\n");
}


int main(int argc, char *argv[]) {
    if (argc != 3 || strcmp(argv[1], "-m") != 0) {
        fprintf(stderr, "Usage: %s -m <Download|Filter|Combine|Decode>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[2], "Download") == 0) {
        download_and_unzip();
    } else if (strcmp(argv[2], "Filter") == 0) {
        filter_files();
    } else if (strcmp(argv[2], "Combine") == 0) {
        combine_files();
    } else if (strcmp(argv[2], "Decode") == 0) {
        decode_rot13();
    } else {
        fprintf(stderr, "Invalid mode. Use Download|Filter|Combine|Decode.\n");
        return 1;
    }

    return 0;
}
