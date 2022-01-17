#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>

int mode_l;
int mode_R;

void makeformat(int depth){
    for(int i = 0; i < depth; i++) {
        if(i % 4 == 0) printf("|");
        else printf(" ");
    }
}

void printmode(__mode_t mode){
    /* print type */
    if(S_ISDIR(mode)) printf("d");   // Directory
    if(S_ISREG(mode)) printf("-");   // Regular
    if(S_ISLNK(mode)) printf("l");   // Link
    if(S_ISFIFO(mode)) printf("p");  // Pipe
    if(S_ISBLK(mode)) printf("b");   // Block
    if(S_ISCHR(mode)) printf("c");   // Char
    /* print permission */
    if(mode & S_IRUSR) printf("r");
    else printf("-");
    if(mode & S_IWUSR) printf("w");
    else printf("-");
    if(mode & S_IXUSR) printf("x");
    else printf("-");
    if(mode & S_IRGRP) printf("r");
    else printf("-");
    if(mode & S_IWGRP) printf("w");
    else printf("-");
    if(mode & S_IXGRP) printf("x");
    else printf("-");
    if(mode & S_IROTH) printf("r");
    else printf("-");
    if(mode & S_IWOTH) printf("w");
    else printf("-");
    if(mode & S_IXOTH) printf("x");
    else printf("-");
    printf(" ");
    return;
}

void printowner(struct stat statbuf){
    struct passwd *pw;
    struct group *gr;
    if((pw = getpwuid(statbuf.st_uid)) == NULL) printf("Pwuid get failed! ");
    else printf("%s ", pw->pw_name);
    if((gr = getgrgid(statbuf.st_gid)) == NULL) printf("Grgid get failed! ");
    else printf("%s ", gr->gr_name);
    return;
}

int printdir(char *dir, int depth){
    makeformat(depth);  // make output format
    printf("currDepth: %d\n", depth);
    DIR *dp;    // imformation of current directory
    struct dirent *entry;   // information of an entry
    struct stat statbuf;    // state imformation of an entry
    if  ((dp = opendir(dir)) == NULL){  // try to open current directory
        printf("Directory open failed!\n");
        return -1;
    }
    if(chdir(dir) == -1){   // change derectory to current
        printf("Directory change failed!\n");
        return -1;
    }
    while(entry = readdir(dp)){
        if(entry->d_name[0] != '.'){
            makeformat(depth);  // make output format
            if(mode_l){
                lstat(entry->d_name, &statbuf); // get state of the entry
                printmode(statbuf.st_mode); // print mode
                printf("%d ", (int)statbuf.st_nlink);   // print linking number
                printowner(statbuf);    // print username and groupname
                printf("%d ", (int)statbuf.st_size);    // print file size
                printf("%.12s ", ctime(&statbuf.st_mtime) + 4);    // print formative time (drop week, second and year)
            }
            printf("%s\n", entry->d_name);
            if(entry->d_type == 4 && mode_R){ // dir type
                if(printdir(entry->d_name, depth+4) == -1){
                    printf("Subdirectory read failed!\n");
                }
                chdir("../"); // return to parent directory
            }
        }

    }

    return 0;
}

int main(int argc, char* argv[]){
    if(argc == 1){
        printdir(".", 0);
    }
    else if(argc == 2){
        if(argv[1][0] != '-'){
            printdir(argv[1], 0);
        }
        else{
            if(argv[1][0] != '-'){
                printf("Arg error!\n");
                return -1;
            }
            for(int i = 1; argv[1][i]; i++){
                if(argv[1][i] == 'l') mode_l = 1;
                if(argv[1][i] == 'R') mode_R = 1;
            }
            printdir(".", 0);
        }
    }
    else if(argc == 3){
        if(argv[2][0] != '-'){
            printf("Arg error!\n");
            return -1;
        }
        for(int i = 1; argv[2][i]; i++){
            if(argv[2][i] == 'l') mode_l = 1;
            if(argv[2][i] == 'R') mode_R = 1;
        }
        printdir(argv[1], 0);
    }
    else{
        printf("Arg error!\n");
    }
    return 0;
}
