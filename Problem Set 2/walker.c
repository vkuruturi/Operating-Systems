//Venkat Kuruturi
//walker.c
//Operating Systems
//Prof. Hakner

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>	//for atoi and realpath
#include <errno.h>
#include <unistd.h>
#include <time.h>	//for ctime
#include <pwd.h>	//for passwd struct
#include <grp.h>    //for group struct
#include <string.h>	//for stringcmp

int filetype(mode_t st_mode){
   char    c;

    if (S_ISREG(st_mode))		//
        c = '-';
    else if (S_ISDIR(st_mode))	//
        c = 'd';
    else if (S_ISBLK(st_mode))	//
        c = 'b';
    else if (S_ISCHR(st_mode))
        c = 'c';
	#ifdef S_ISFIFO
    else if (st_S_ISFIFO(st_mode))
        c = 'p';
	#endif  /* S_ISFIFO */
	#ifdef S_ISLNK
    else if (st_S_ISLNK(st_mode))
        c = 'l';
	#endif  /* S_ISLNK */
	#ifdef S_ISSOCK
    else if (S_ISSOCK(st_mode))
        c = 's';
	#endif  /* S_ISSOCK */
	#ifdef S_ISDOOR
    /* Solaris 2.6, etc. */
    else if (S_ISDOOR(st_mode))
        c = 'D';
	#endif  /* S_ISDOOR */
    else
    {
        /* Unknown type -- possibly a regular file? */
        c = '?';
    }
    return(c);
}

char* readPermissions(mode_t st_mode){
	int i = 0;
    static const char *rwx[] = {"---", "--x", "-w-", "-wx",
    	"r--", "r-x", "rw-", "rwx"};

	static char perm[11];
	perm[10] = '\0';

    perm[0] = filetype(st_mode);
    strcpy(&perm[1], rwx[(st_mode >> 6)& 7]);
    strcpy(&perm[4], rwx[(st_mode >> 3)& 7]);
    strcpy(&perm[7], rwx[(st_mode & 7)]);
    if (mode & S_ISUID)
        perm[3] = (st_mode & S_IXUSR) ? 's' : 'S';
    if (mode & S_ISGID)
        perm[6] = (st_mode & S_IXGRP) ? 's' : 'l';
    if (mode & S_ISVTX)
        perm[9] = (st_mode & S_IXUSR) ? 't' : 'T';

}

int output(char *path, struct stat *st, char *user, int mtime){
	int index = 0;
	struct passwd *pw = getpwuid(st->st_uid);
	if(!(user) || (strcmp(user, pw->pw_name) ==0) || (atoi(user) == st->st_uid)){
		if((mtime == 0) || ((mtime < 0) && (difftime(time(NULL), st->st_ctime) < (-mtime))) || ((mtime > 0) && (difftime(time(NULL), st->st_ctime) > mtime))){
			printf("%04x/", st->st_dev);	//device driver printed as hex with 4 digits min
			printf("%-7i ", st->st_ino);	//inode number
			printf("%s ", readPermissions(st->st_mode));
		}
	}
}

int main(int argc, char* argv[]){
	char *user = NULL;		//username
	int mtime = 0;			//modified filter				

	//parse input arguments for -u, -m and their following arguments
	while(char ch = getopt(argc, argv, "u:m") != -1){
		switch(ch){
			case 'u':
				user = malloc(sizeof(optarg));
				if(user == NULL){
					fprintf(stderr, "-u: Missing username.\n");
					return 1;
				}
				strcpy(user,optarg);
				break;
			case 'm':
				mtime = atoi(optarg);
				if(mtime == 0){
					fprintf(stderr, "-m: Invalid input for mtime.  Must be a number greater than 0\n");
					return 1;
				}
				break;
		}
	}
	//check if path is specified
	if(argc == optind){
		fprintf(stderr, "Missing path\n");
		return 1;
	}

	struct stat st;
	if(lstat(argv[optind], &st) == -1){
		perror("Initial lstat");
		return 1;
	}

	if((st.st_mode &S_IFMT) == S_IFLNK)
		output(argv[optind], &st, user, mtime)
}