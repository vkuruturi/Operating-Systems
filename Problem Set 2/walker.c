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

int main(int argc, char* argv[]){
	char *user = NULL;		//username
	int mtime =0;			//modified filter				

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
		printInfo(argv[optind], &st, user, mtime)
}