#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <glob.h>

int main() {
    DIR* directory;
    struct dirent* inp;
    glob_t glob_struct;
    int glob_res;
    char** matched;
    char pattern[BUFSIZ];
    int flag = 0;
    fgets(pattern, BUFSIZ, stdin);

    if (pattern[strlen(pattern) - 1] == '\n') {
        pattern[strlen(pattern) - 1] = '\0';
    }

    directory = opendir(".");
    if (directory == NULL) {
        perror("Failed to open directory");
        return -1;
    }

    glob_res = glob(pattern, 0, NULL, &glob_struct);
    if (glob_res == GLOB_NOMATCH) {
        printf("No files found with pattern: %s\n", pattern); 
        return -1;
    }
    
    matched = glob_struct.gl_pathv;
    while (*matched) {
        printf("%s\n", *matched);
        matched++;
    }
    globfree(&glob_struct);
    closedir(directory);
    return 0;
}
