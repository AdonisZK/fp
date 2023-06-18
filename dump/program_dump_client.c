#include <proc_service.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <pthread.h>
#include <ctype.h>
#include <fuse.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <linux/limits.h>
#include <limits.h>

int main(void)
{
    // Inisialisasi
    DIR *dp;
    FILE *fp;
    struct dirent *ep;
    char filename[101], strfile[101], current[PATH_MAX];
    char *modifyname = strstr(filename, ";");
    int line = 0;

    // Mendapatkan current working directory
    getcwd(current, sizeof(current)); 

    // Menulis string ke variabel
    sprintf(strfile, "%s/databases/%s", current, modifyname, filename);

    // buka directory
    dp = opendir(strfile);

    // check jika tidak null
    if (dp != NULL) {
        bool check = true;
        while (check) {
            char table[101], text[101];
            fp = fopen(readdir(dp)->d_name, "r");

            fgets(text, 101, fp);

            while ((modifyname = fgetc(fp)) != EOF)
            {
                if (modifyname == '\n'){
                    line++;
                } else if (line == 1) {
                    break;
                }
            }
            printf("DROP TABLE %s;", readdir(dp)->d_name);
            printf("CREATE TABLE %s (%s);", readdir(dp)->d_name, text);

            while ((modifyname = fgetc(fp)) != EOF && (modifyname != '\n')) {
                printf("INSERT INTO %s (%s);", readdir(dp)->d_name, modifyname);
            }
            fclose(fp);
        }
        (void)closedir(dp);
    } else {
        perror("Error to open and read");
    }
    return 0;
}