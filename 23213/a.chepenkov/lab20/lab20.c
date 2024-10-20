#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <glob.h>

int errfunc(const char* epath, int errno){
    return 0;
}

int main() {
    glob_t glob_struct;
    int glob_res;
    char** matched;
    char pattern[BUFSIZ];
    int flag = 0;
    fgets(pattern, BUFSIZ, stdin);

    if (pattern[strlen(pattern) - 1] == '\n') {
        pattern[strlen(pattern) - 1] = '\0';
    }

    glob_res = glob(pattern, 0, &errfunc, &glob_struct);
    if (glob_res == GLOB_NOMATCH) {
        printf("No files found with pattern: %s\n", pattern);
        return -1;
    }
    if (glob_res != 0) {
        fprintf(stderr, "An error occured while glob function\n");
        return -1;
    }

    matched = glob_struct.gl_pathv;
    while (*matched) {
        printf("%s\n", *matched);
        matched++;
    }
    globfree(&glob_struct);
    return 0;
}
