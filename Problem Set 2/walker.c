/*Venkat Kuruturi
walker.c
Operating Systems
Prof. Hakner

Compiled using the gnu99 standard because lstat
*/
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
#include <getopt.h>

//function to determine filetype
char filetype(mode_t st_mode){
   char c;
    switch (st_mode & S_IFMT)
    {
        case S_IFREG:
            c = '-';		//regular file
            break;
        case S_IFDIR:
            c = 'd';		//directory
            break;
        case S_IFCHR:
            c = 'c';		//character file
            break;
        case S_IFBLK:
            c = 'b';		//block file
            break;
        case S_IFLNK:
            c = 'l';		//symbolic link
            break;
        case S_IFSOCK:
            c = 's';		//network socket
            break;
        case S_IFIFO:
            c = 'p';		//fifo
            break;
        default:
        	c = '?'; 		// unknown filetype
        	break;
    }
    return c;
}

//read permissions of the node
char* readPermissions(mode_t st_mode){
	int i = 0;
    static const char *rwx[] = {"---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"};

	static char perm[11];
	perm[10] = '\0';
	char *pc = perm;

    perm[0] = filetype(st_mode);
    strcpy(&perm[1], rwx[(st_mode >> 6)& 7]);
    strcpy(&perm[4], rwx[(st_mode >> 3)& 7]);
    strcpy(&perm[7], rwx[(st_mode & 7)]);
    if (st_mode & S_ISUID)
        perm[3] = (st_mode & S_IXUSR) ? 's' : 'S';
    if (st_mode & S_ISGID)
        perm[6] = (st_mode & S_IXGRP) ? 's' : 'S';
    if (st_mode & S_ISVTX)
        perm[9] = (st_mode & S_IXUSR) ? 't' : 'T';

    return pc;
}

//use readLink to dereference symbolic link
char* getLink(char *path, struct stat *st){
	char symLink[4096];
	char *pc = symLink;	
	if(symLink == NULL){
		fprintf(stderr, "malloc failed to allocate symLink string.");
		exit(EXIT_FAILURE);
	}
	int r = readlink(path, symLink, st->st_size + 1);
	if(r<0){
		fprintf(stderr,"readLink() failed.\n");
		exit(EXIT_FAILURE);
	}

	symLink[r] = '\0';
	return pc;
}

//print out info
int output(char *path, struct stat *st, char *user, int mtime){
	struct passwd *pw = getpwuid(st->st_uid);
	//first if statement: if no user is defined, or if user matches owner name or uid
	if(!(user) || (strcmp(user, pw->pw_name) ==0) || (atoi(user) == st->st_uid)){
		//and if mtime isn't difined or if it is defined and the file is in range, print info
		if((mtime == 0) || ((mtime < 0) && (difftime(time(NULL), st->st_ctime) < (-mtime))) || ((mtime > 0) && (difftime(time(NULL), st->st_ctime) > mtime))){
			printf("%04x/", st->st_dev);				//device driver printed as hex with 4 digits min
			printf("%-7i ", st->st_ino);				//inode number
			printf("%s ", readPermissions(st->st_mode));//permissions
			printf("%2i", st->st_nlink);
			if (printf("%10s ", pw->pw_name) <= 0)		//try to print user name
				printf("%10i ", pw->pw_uid);			//if it fails, print uid
			struct group *grp = getgrgid(st->st_gid);
			if(!grp){
				perror("getgrgid failed.");
				return -1;
			}
			if (printf("%5s ", grp->gr_name) <= 0)		//try to print group name
				printf("%5i ", grp->gr_gid);			//if it fails, print gid
			if (((st->st_mode & S_IFMT) == S_IFCHR) || ((st->st_mode & S_IFMT) == S_IFBLK))
				printf("%10x ", st->st_rdev);			//print device id
			else
				printf("%10i ", st->st_size);			//print size

			printf("%s ", ctime(&(st->st_mtime)));		//modified time
			printf("%s ", path);						//path of the file
			//if the node is a symlink, then display what it's linked to
			if ((st->st_mode & S_IFMT) == S_IFLNK)
				printf("-> %s", getLink(path, st));
			printf("\n");
			return 0;
		}
	}
	return 0;
}

int ls(char *path, struct stat *st, char *user, int mtime){
	DIR *dirp;
	struct dirent *d;
	if(output(path,st, user, mtime) == -1)
		return -1;
	if((dirp = opendir(path)) == NULL){
		fprintf(stderr, "Unable to open directory at path %s: %s\n",path, strerror(errno));
		return -1;
	}
	while(d = readdir(dirp)){
		//if name = "." or "..", ignore because that would lead to infinite loop
		if(strcmp(".", d->d_name) && strcmp("..", d->d_name)){
			struct stat st_new;
			char *fullpath = malloc(sizeof(path) +sizeof(d->d_name) + 2);	//size = current dir length + name length + / + \0
		
			if(fullpath == NULL){
				fprintf(stderr, "Malloc failed to allocate space for path variable");
				free(fullpath);
				return -1;
			}
			sprintf(fullpath, "%s/%s",path, d->d_name);
			if(lstat(fullpath, &st_new) == -1){
				perror("Error using lstat.");
				free(fullpath);
				return -1;
			}
			if((st_new.st_mode & S_IFMT) == S_IFDIR){
				if(ls(fullpath,&st_new,user,mtime) == -1){
				free(fullpath);
					return -1;
				}
			}
			else{
				if(output(fullpath,&st_new,user,mtime) == -1){
					free(fullpath);
					return -1;
				}
			}
			free(fullpath);
		}

	}
	if(closedir(dirp) == -1){
		perror("error using closedir.");
		return -1;
	}
	return  0;
}

int main(int argc, char* argv[]){
	char *user = NULL;		//username
	int mtime = 0;			//modified filter				
	char ch;
	//parse input arguments for -u, -m and their following arguments
	while((ch = getopt(argc, argv, "u:m:")) != -1){
		switch(ch){
			case 'u':
				user = malloc(sizeof(optarg));
				if(user == NULL){
					fprintf(stderr, "-u: Missing username.\n");
					return EXIT_FAILURE;
				}
				strcpy(user,optarg);
				break;
			case 'm':
				mtime = atoi(optarg);
				if(mtime == 0){
					fprintf(stderr, "-m: Invalid input for mtime.  Must be a number greater than 0\n");
					return EXIT_FAILURE;
				}
				break;
		}
	}
	//check if path is specified
	if(argc == optind){
		fprintf(stderr, "Missing path\n");
		return EXIT_FAILURE;
	}

	struct stat st;
	if(lstat(argv[optind], &st) == -1){
		perror("Initial lstat");
		return EXIT_FAILURE;
	}
	//if node is a symbolic link, do not dereference link. just print info
	if((st.st_mode & S_IFMT) == S_IFLNK){
		if(output(argv[optind], &st, user, mtime)){
			return EXIT_FAILURE;
		}
	}
	else{	//walk through
		if(ls(argv[optind], &st, user, mtime))
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}