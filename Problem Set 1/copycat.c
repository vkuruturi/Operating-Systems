#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>


int cat(const int fd_out,const int fd_in, char* buffer, size_t buffersize){
	int n = 0;
	int length = 0;
	int write_length = 0;
	while((length = read(fd_in, buffer, buffersize))!= 0){
		if(length < 0){
			fprintf(stderr, "Error reading input: %s\n", strerror(errno));
			free(buffer);
			return -1;
		}
		else{
			write_length = 0;
			while(write_length<length){
				n = write(fd_out, buffer+ write_length, length - write_length);
				if(n < 0){
					fprintf(stderr, "Can't write to output file %d: %s", fd_out, strerror(errno));
					return -1;
				}
				write_length += n;
			}
		}
	}
	return 0;
}

void close_program(const int fd_out, const int fd_in, char* buffer){
	if(fd_out != STDOUT_FILENO)
		close(fd_out);
	if(fd_in != STDIN_FILENO)
		close(fd_in);
	free(buffer);
}

int main(int argc, char **argv){
	
	size_t buffersize = 1024;
	short b_loc = -1;		//value of argc where -b is found
	short o_loc = -1;		//value of argc where -o is found
	int fd_in = STDIN_FILENO;
	int fd_out = STDOUT_FILENO;
	char *buffer;
	int bo_count=0;

	//PARSE INPUT ARGUMENTS. If multiple arguments for buffer size and output file are specified, throw error
	printf("Parsing input arguments...\n");
	for(int i = 1; i < argc-1; i++){
		//Check for buffer size in arguments
		if((strcmp("-b",argv[i]) == 0) || (strcmp("-B", argv[i]) == 0)){
			printf("found buffersize: %s\n", argv[i+1]);
			if(b_loc == -1){
				printf("%d\n", (int)strlen(argv[i+1]));
				if((buffersize = atoi(argv[i+1]) % ((int) pow(10,strlen(argv[i+1])))) ==0 ){
					printf("Invalid input arguments.  Buffer size must be a number greater than 0.\n");
					close_program(fd_out, fd_in, buffer);
					return -1;
				}
				b_loc = i;
				bo_count +=2;
			}
			else{ 
				printf("Invalid input arguments.  More than one buffer size specified.\n");
				return -1;
			}
		}
		//Check for output file name in arguments and open it.
		if((strcmp("-o",argv[i]) == 0) || (strcmp("-O", argv[i]) == 0)){
			printf("found output file name: %s\n", argv[i+1]);
			if( o_loc == -1){
				o_loc = i;
				bo_count += 2;
				if((fd_out = open(argv[i+1], O_WRONLY|O_CREAT|O_TRUNC, 0777))<0){
		            if (errno == ENOENT)
        		        fprintf(stderr, "Sorry, %s does not appear to be a valid file name.\n", argv[i+1]);
    		        else
    		        	fprintf(stderr, "Cannot open file %s for reading: %s\n", argv[i+1], strerror(errno));
            		close_program(fd_out, fd_in, buffer);
            		return -1;
				}
			}
			else{
				printf("Invalid input arguments.  More than one output file specified.\n");
				close_program(fd_out, fd_in, buffer);
				return -1;
			}
		}
	}
	printf("allocating buffer\n");
    if ((buffer = (char*)malloc(buffersize)) == 0){
        fprintf(stderr, "Could not allocate %zu bytes of data to buffer size with malloc", buffersize);
        return -1;
    }

	if(argc ==1 || argc == bo_count +1){
		if (cat(fd_out, fd_in, buffer, buffersize) == -1){
			close_program(fd_out, fd_in, buffer);
			return -1;
		}
		else return EXIT_SUCCESS;
	}
	else{
		for(int i = 1; i< argc; i++){
			if (i== b_loc || i == o_loc)
				i++;
			else{
				if(fd_out = open(argv[i+1], O_RDONLY) < 0){
                    if (errno == ENOENT)
                        fprintf(stderr, "Sorry, %s does not appear to be the name of a valid file.\n", argv[i]);
                    else
                    	fprintf(stderr, "Can't open file %s for reading: %s\n", argv[i], strerror(errno));
                    close_program(fd_out, fd_in, buffer);
                    return -1;
				}
				if (cat(fd_out, fd_in, buffer, buffersize) == -1){
					close_program(fd_out, fd_in, buffer);
					return -1;
				}
				close(fd_in);
			}
		}
	}

	close_program(fd_out, fd_in, buffer);
	return EXIT_SUCCESS;

}