//Venkat Kuruturi
//Pset 4
//catgrepmore

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/resource.h>


int files_processed = 0;
int bytes_processed = 0;
pid_t pid_grep, pid_more;
char buffer[4096];
size_t buffsz = 4096;

void sigint_handler(int sig){
	fprintf(stderr,"Processed %d files.\n", files_processed);
	fprintf(stderr,"Processed %d bytes of data.\n", bytes_processed);
	exit(1);
}


int cat(int infd, int outfd){
	int n = 0, length = 0, w_length;
	char *buff = &buffer[0];
    while ((length = read(infd, buff, buffsz)) != 0){
        if (length < 0){
            fprintf(stderr, "Error reading input: %s\n", strerror(errno));
            return -1;
        }
        else{
            w_length = 0;
            while (w_length < length){
                n = write(outfd, buff+w_length, length-w_length);
                bytes_processed += n;
                w_length += n;
            }
        }
    }
    return 0;
}

//opens 2 pipes, forks twice and exec's grep and more
void process_file(char *pattern,char *pathname){
	int p_grep[2], p_more[2];
	int infd;

	//create pipes for more and grep
	if(pipe(p_more) <0){
		perror("Error creating pipe for more");
		exit(1);
	}
	if(pipe(p_grep) <0){
		perror("Error creating pipe for grep");
		exit(1);
	}

	//fork for grep
	if((pid_grep = fork()) < 0){
		perror("Failed to fork for grep");
		exit(1);
	}
	else if(pid_grep == 0){
		if(close(p_grep[1]) < 0 || close(p_more[0])  < 0){
			fprintf(stderr, "Error closing  unused pipes in grep: %s\n", strerror(errno));
			exit(1);
		}

		//redirect stdin to read from p_grep and stdout to write to p_more
		if((dup2(p_grep[0], 0) < 0) || (dup2(p_more[1], 1) < 0)){
			perror("dup2 failed in grep");
			exit(1);
		}

		if((close(p_grep[0]) < 0) || (close(p_more[1]) < 0)){
			fprintf(stderr, "Error closing file descriptors for pipes in grep: %s\n", strerror(errno));
			exit(1);
		}
		execlp("grep", "grep", pattern, NULL);
		perror("execlp failed in grep");
		exit(1);
	}

	//fork for more
	if((pid_more = fork()) < 0){
		perror("Failed to fork for more");
		exit(1);
	}
	else if(pid_more == 0){
		if(close(p_grep[0]) < 0 || close(p_grep[1]) < 0 || close(p_more[1]) < 0){
			fprintf(stderr, "Error closing fd's for pipes in more: %s\n", strerror(errno));
			exit(1);
		}
		//redirect stdin to read from p_more
		if(dup2(p_more[0], 0) < 0){
			perror("dup2 failed in more");
			exit(1);
		}

		if(close(p_more[0]) < 0){
			fprintf(stderr, "Error closing read end of p_more in more: %s\n",strerror(errno));
			exit(1);
		}
		execlp("more", "more", NULL);
		perror("execlp failed in more");
		exit(1);
	}

	//in parent process
	if(pid_more > 0 && pid_grep > 0){
		//close pipe fd's that aren't necessary
		if(close(p_more[0]) < 0 || close(p_grep[0]) < 0 || close(p_more[1]) < 0){
			fprintf(stderr, "Error closing pipes in parent process: %s\n", strerror(errno));
			exit(1);
		}
		//open file for reading
		if((infd = open(pathname, O_RDONLY)) < 0){
			fprintf(stderr, "Error opening file %s for reading: %s\n", pathname, strerror(errno));
			exit(1);
		}
		//read file andwrite to pipe.  if there's an error, check if EPIPE occured.
		//if EPIPE, close fdin and continue, else exit
		if (cat(infd, p_grep[1]) == -1)
			if (errno != EPIPE)
				exit(1);
		if(close(p_grep[1]) < 0){
			fprintf(stderr, "Error closing write end of p_grep: %s\n", strerror(errno));
			exit(1);
		}
		//wait
		int stat1, stat2;
		waitpid(pid_grep, &stat1, 0);
		waitpid(pid_more, &stat2, 0);
		if(close(infd) < 0){
			fprintf(stderr, "Error closing input file: %s\n", strerror(errno));
			exit(1);
		}
	}

}

int main(int argc, char* argv[]){
	signal(SIGPIPE, sigpipe_handler);
	signal(SIGINT, SIG_IGN);
	
	if(argc < 3){
		fprintf(stderr,"Not enough arguments.\n");
		exit(1);
	}
	for(int i = 2; i <argc; i++){
		process_file(argv[1], argv[i]);
		files_processed++;
	}
	return 0;
}