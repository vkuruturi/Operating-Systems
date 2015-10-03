//Venkat Kuruturi
//Pset 4
//simple shell

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>

#define MAX_LEN 1000  //maximum length of an input line

int reachedEOF = 0;
int scriptBool = 0;
char line[MAX_LEN];
char* my_argv[MAX_LEN/2];		//holds maximum of 512 arguments
char* my_argv1[MAX_LEN/2];		//holds redirection arguments (>>filename, etc.)
char line[MAX_LEN];
int my_argc;
int numrs;
struct timeval start, end;
pid_t pid;

//chops the first s characters from the string str and returns the remainder
void chop_char(char *str, size_t s){
	size_t s_len = strlen(str);
	if(s >= s_len){
		fprintf(stderr, "can't remove redirect operator.\n");
		exit(1);
	}
	memmove(str, str+s, s_len - s +1);
}

//compare 2 strings.  return 2 if they match exactly
//return 1 if sym matches the str's frist characters
// return 0 if if sym does not match with str
int compare(const char *str,const char *sym){
	for( ; *str == *sym; str++, sym++){
		if(*sym == '\0')
			return 2;
	}

	if( *sym == '\0')
		return 1;
	
	return 0;
}

//checks to see if string contains any of the redirect operators
//returns appropriate value for switch case in do_command()
int redirect_check(const char *str){
	int i = 0;
	if((i = compare(str, "<")))
		return i;
	if((i = compare(str, ">>")))
		return i + 6;
	if((i = compare(str, "2>>")))
		return i + 8;
	if((i = compare(str, ">")))
		return i + 2;
	if((i = compare(str, "2>")))
		return i + 4;
	return 0;
}

void mygetline(){
	int i;
	//read fd 0 until a newline char is encountered or more than MAX_LEN
	//characters have been read
	for (i = 0;(i != MAX_LEN) && (line[i]=getchar())!='\n'; i++){
        if (line[i] == EOF){
        	if (scriptBool){
        		reachedEOF = 1;
        		break;
        	}
        	else{
        		printf("\n");
        		exit(0);
        	}
        }
    }

    if(i == MAX_LEN){
    	fprintf(stderr, "input line too long.\n");
    	exit(1);
    }
    line[i] = '\0';

    my_argv[0] = strtok(line, " \t");
    printf("%s ", my_argv[0]);
    my_argc = 1;
    numrs = 0;
    my_argv[my_argc] = strtok(NULL, " \t");
    printf("%s ", my_argv[my_argc]);

    //add all arguments to my_argv until the first argument
    //with a redirect operator is encountered
    while ( (my_argv[my_argc]) != NULL && redirect_check(my_argv[my_argc]) == 0){
		++my_argc;
		my_argv[my_argc] = strtok(NULL, " \t");
		printf("%s ", my_argv[my_argc]);
	}
	printf("\n\n");
	//add redirect operator arguments to my_argv1
    if (my_argv[my_argc] != NULL && redirect_check(my_argv[my_argc]) != 0){
        my_argv1[numrs] = my_argv[my_argc];
        printf("%s ", my_argv[my_argc]);
        printf("%s ", my_argv1[numrs]);
        my_argv[my_argc] = NULL;
        ++numrs;
        my_argv1[numrs] = strtok(NULL, " \t");
        while (my_argv1[numrs] != NULL)
        {
            ++numrs;
            my_argv1[numrs] = strtok(NULL, " \t");
        }
    }
}

void printProcessInfo(){
	struct rusage ru;
	int status;
	pid_t w = wait3(&status,0,&ru);
	if(w == -1)
		perror("wait3 failed");
	else{
		if(gettimeofday(&end, NULL) <0)
			perror("end gettimeofday");

		printf("Executing command %s with arguments", my_argv[0]);
		for(int i =1 ; i != my_argc; i++)
			printf(" \"%s\"", my_argv[i]);
		printf("\n");

		if(status !=0){
			if(WIFSIGNALED(status))
				printf("Exited with signal %d\n", WTERMSIG(status));
			else
				printf("Exited with return code %d,\n", WEXITSTATUS(status));
		}

		int seconds, microseconds;
		if((microseconds = end.tv_usec - start.tv_usec)<0)
			microseconds += 1000000;
		seconds = end.tv_sec - start.tv_sec;

		printf("consuming %1u.%.6u real seconds, %1u.%.6u user, %1u.%.6u system\n",
            end.tv_sec - start.tv_sec,
            end.tv_usec - start.tv_usec,
            ru.ru_utime.tv_sec, ru.ru_utime.tv_usec,
            ru.ru_stime.tv_sec, ru.ru_stime.tv_usec);
	}
}

void redirect(char *pathname, int stream, int redirect_type){
	int fd;
	if (redirect_type == 0)			// <
		fd = open(pathname, O_RDONLY);
	else if (redirect_type == 1)	// >
		fd = open(pathname, O_WRONLY|O_CREAT|O_TRUNC, 0666);
	else if (redirect_type == 2)	// >>
		fd = open(pathname, O_WRONLY|O_CREAT|O_APPEND, 0666);
	if(fd<0){
		fprintf(stderr, "Can't open file %s.", pathname);
		perror("");
		exit(1);
	}

	if (dup2(fd,stream) < 0)
	{
		fprintf(stderr, "Can't dup2 %s to stream %i", pathname, stream);
		perror("");
		exit(1);
	}

	if(close(fd) <0){
		perror("error closing fd");
		exit(1);
	}
}

int do_command(){
	if(gettimeofday(&start, NULL) <0)
		perror("start gettimeofday");
	if((pid = fork()) == -1){
		perror("Fork ");
		exit(1);
	}

	if(pid == 0){
		int i;

		// I/O Redirection
		if(numrs != 0){
			for(i = 0; i!= numrs; ++i){
				
				switch(redirect_check(my_argv1[i])){
					case 1:
						chop_char(my_argv1[i], 1); 		// "<filename"
						redirect(my_argv1[i], 0, 0);
						break;
					case 2: 
						redirect(my_argv1[i +1], 0, 0); // "< filename"
						break;
					case 3:
						chop_char(my_argv1[i], 1);		// ">filename"
						redirect(my_argv1[i], 1, 1);
						break;
					case 4:								//"> filename"
						redirect(my_argv1[i+1], 1, 1);
                    	break;
					case 5:								//"2>filename"
						chop_char(my_argv1[i], 2);
                    	redirect(my_argv1[i], 2, 1);
                    	break;
					case 6:								//"2> filename"
						redirect(my_argv1[i+1], 2, 1);
                    	break;
					case 7:								//">>filename"
						chop_char(my_argv1[i], 2);
                    	redirect(my_argv1[i], 1, 2);
                    	break;
					case 8:								//">> filename"
						redirect(my_argv1[i+1], 1, 2);
                    	break;
					case 9:								//"2>>filename"
						chop_char(my_argv1[i], 3);
                    	redirect(my_argv1[i], 2, 2);
                    	break;
					case 10:							//"2>> filename"
						redirect(my_argv1[i+1], 2, 2);
                    	break;
				}
			}
		}
		execvp(my_argv[0], my_argv);
		perror("execvp");
		exit(1);
	}

	if(pid != 0)
		printProcessInfo();
	return 0;

}

int main(int argc, char* argv[]){

	if(argc >= 2){
		scriptBool = 1;
		redirect(argv[1], 0 ,0);
	}

	while(1){
		if(!scriptBool)
			printf("$ ");
		printf("gettingline\n");
		mygetline();
		if(!(my_argv[0] == NULL || compare(my_argv[0], "#")))
			do_command();
		if (reachedEOF){
			printf("\n");
			return 0;
		}
	}
	return 0;
}