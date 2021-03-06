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
//eg. ">fname" compared with ">" returns 1
// return 0 if if sym does not match with str
int compare(const char *str,const char *sym){
	while(*str == *sym){
		if(*sym == '\0')
			return 2;
		str++;
		sym++;
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

//reads one line from stream 0.  parses it into normal args and redirection args
//normal args are stored in my_argv, redirection args are stored in my_argv1
void mygetline(){
	int i;
	//read stream 0 until a newline char is encountered or more than MAX_LEN
	//characters have been read.  If EOF is read, exit if not a script, or break
	//from loop if it is a script
	for (i = 0;(i != MAX_LEN) && (line[i]=getchar())!='\n'; i++){
        if (line[i] == EOF){

        	//if processing a script, break from for loop and process last line
        	if (scriptBool){
        		reachedEOF = 1;
        		break;
        	}
        	//if not a script, exit
        	else{
        		printf("exiting...\n");
        		exit(0);
        	}
        }
    }

    if(i == MAX_LEN){
    	fprintf(stderr, "input line too long.\n");
    	exit(1);
    }
    line[i] = '\0';

    //break each word into it's own string
    my_argv[0] = strtok(line, " \t");
    my_argc = 1;
    numrs = 0;
    my_argv[my_argc] = strtok(NULL, " \t");


    //add all arguments to my_argv until the first argument
    //with a redirect operator is encountered
    while ( (my_argv[my_argc]) != NULL && redirect_check(my_argv[my_argc]) == 0){
		++my_argc;
		my_argv[my_argc] = strtok(NULL, " \t");
	}

	//add redirect operator arguments to my_argv1, 
    if (my_argv[my_argc] != NULL && redirect_check(my_argv[my_argc]) != 0){
        my_argv1[numrs] = my_argv[my_argc];
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


//prints info about the child process
void printProcessInfo(){
	struct rusage ru;
	int status;
	pid_t w = wait3(&status,0,&ru);
	if(w == -1)
		perror("wait3 failed");
	else{
		if(gettimeofday(&end, NULL) <0)
			perror("end gettimeofday");

		if (WIFEXITED(status))
        	printf("exited with return code %d\n", WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
        	printf("killed by signal %d\n", WTERMSIG(status));
        else if (WIFSTOPPED(status))
            printf("stopped by signal %d\n", WSTOPSIG(status));
        else if (WIFCONTINUED(status))
            printf("continued\n");
        
		int seconds, microseconds;
		if((microseconds = end.tv_usec - start.tv_usec)<0)
			microseconds += 1000000;
		seconds = end.tv_sec - start.tv_sec;

		printf("consuming %1lld.%.6lld real seconds, %1lld.%.6lld user, %1lld.%.6lld system\n",
            (long long)(end.tv_sec - start.tv_sec),
            (long long)(end.tv_usec - start.tv_usec),
            (long long)ru.ru_utime.tv_sec, (long long)ru.ru_utime.tv_usec,
            (long long)ru.ru_stime.tv_sec, (long long)ru.ru_stime.tv_usec);
	}
}

//function to redirect streams.
//redirect type 0 -> '<', type 1 -> '>', type 2 -> '>>'
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
		fprintf(stderr, "Error closing fd to file %s:", pathname);
		perror("");
		exit(1);
	}
}

//processes command by forking, redirecting i/o in the child process,
//and using execvp in the child process 
int do_command(){
	
	printf("Executing command %s", my_argv[0]);
	if(my_argc > 1)
		printf(" with arguments");
	for(int i =1 ; i < my_argc; i++)
		printf(" \"%s\"", my_argv[i]);
	printf("\n");

	if(gettimeofday(&start, NULL) <0)
		perror("start gettimeofday");
	if((pid = fork()) == -1){
		perror("Fork ");
		exit(1);
	}
	
	//actions to be performed in child process
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

	//actions to be performed in parent process
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
		
		mygetline();
		//ignore line if it starts with # or if it is empty
		if(!(my_argv[0] == NULL || compare(my_argv[0], "#")))
			do_command();
		
		if (reachedEOF){
			printf("end of file\n");
			return 0;
		}
	}
	return 0;
}