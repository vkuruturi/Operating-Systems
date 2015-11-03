//Venkat Kuruturi
//Pset 5

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <signal.h>

int fd, rb;
char *testfile = "/tmp/test.txt";
char *map;
struct stat s;
char buffer[1024];
int sz = 100;

void sig_handler(int sig){
	switch(sig)
	{
		case 11:
			fprintf(stderr, "Signal %i (SIGSEGV) encountered. Exiting.\n", sig);
			break;
		case 7:
			fprintf(stderr, "Signal %i (SIGBUS) encountered. Exiting.\n", sig);
			break;
		default:
			fprintf(stderr, "Signal %i encountered. Exiting.\n", sig);	
			break;
	}
	exit(1);
}

void open_file(char *fname){
    printf("Creating file of size %d bytes\n", sz);
    fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if(fd < 0){
        perror("error opening file");
    	exit(1);
    }

    if(lseek(fd, sz-1, SEEK_SET) < 0){
    	perror("error using lseek");
    	exit(1);
    }
    
    if ((write(fd, "", 1)) != 1)
    {
        close(fd);
        perror("Failed to write last byte of the file!");
        exit(1);
    }

    if(fstat(fd, &s) < 0){
	    perror("error using fstat");  	
    	exit(1);
    }

    if(lseek(fd, 0, SEEK_SET) < 0){
    	perror("error using lseek");
    	exit(1);
    }
}

void a(){
	open_file(testfile);
	int size = s.st_size;

	map = mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
	if(map == MAP_FAILED){
		perror("Error using mmap");
		exit(1);
	}

	fprintf(stdout, "Attempting to write to test file.\n");
	sprintf(map, "testwrite");
}

void bc(int val){
	open_file(testfile);
	int size = s.st_size;

    map = mmap(0, size, PROT_READ | PROT_WRITE,
            (val) ? MAP_SHARED : MAP_PRIVATE, fd, 0);
    
    if(map == MAP_FAILED){
    	perror("mmap failed");
    	exit(1);
    }

    fprintf(stdout, "Attempting to write \"testwrite\" to test file.\n");
	sprintf(map, "testwrite");
	if(val == 1)
		fprintf(stdout, "The update should be visible.\n");
	else
		fprintf(stdout, "The update should not be visible.\n");
	printf("printing file below.\n");
	while(( rb = read(fd, buffer, sizeof(buffer))) > 0){
        printf("%s", buffer);
    }
    printf("\n\n");
}

void d(){
	open_file(testfile);
	int size = s.st_size;
	printf("Size of the file before writing to end of mapped region: %d\n", size);

	map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(map == MAP_FAILED){
		perror("mmap failed");
		exit(1);
	}
	printf("Writing \"test write\" to mmaped memory.\n");
	sprintf(map + strlen(map), "test write.");

	if(fstat(fd, &s) <0){
		perror("Error using fstat");
		exit(1);
	}
	int new_size = s.st_size;
	printf("Size of file after writing: %d\n", new_size);
	if(size - new_size == 0)
		printf("Size has not changed.\n");
	else
		printf("Size has changeid.\n");
}

void e(){
	open_file(testfile);
	int size = s.st_size;
	int blah = 10;
	//printf("size of file before write: %d\n", size);
	int i = 0;
	map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(map == MAP_FAILED){
		perror("mmap failed");
		exit(1);
	}
	printf("DISPLAYING 10 BYTES AFTER EOF\n");
	printf("Original file contents (hex):\n");
	for(i = 0; i < blah; i++)
    	printf("<%02X>  ", buffer[i]);
    printf("\n");
    printf("Original mapped memory contents (hex):\n");
    for(i = 0; i < blah; i++)
    	printf("<%02X>  ", map[size+i]);
	printf("\n \n");
	printf("Writing 1 to memory past eof\n");
    map[size] = '1';


    if(lseek(fd, size, SEEK_SET) < 0){
    	perror("error using lseek");
    	exit(1);
    }
	printf("file contents (hex):\n");
	for(i = 0; i < blah; i++)
    	printf("<%02X>  ", buffer[i]);
    printf("\n");
    printf("mapped memory contents (hex):\n");
    for(i = 0; i < blah; i++)
    	printf("<%02X>  ", map[size+i]);

	printf("\n \n");
	if((lseek(fd, blah -1 , SEEK_END)) < 0){
		perror("Error using lseek");
		exit(1);
	}
	printf("writing 2 at 10 past end using write(2)\n");

	if(write(fd, "2", 1) !=1){
		perror("Error writing to file");
		exit(1);
	}

	if((lseek(fd,size, SEEK_SET)) < 0){
		perror("Error using lseek");
		exit(1);
	}

	if((rb = read(fd, buffer, sizeof(buffer))) < 0){
        perror("Error reading file");
    	exit(1);
    }
	printf("file contents (hex):\n");
    for(i = 0; i < blah; i++)
    	printf("<%02X>  ", buffer[i]);
    printf("\n");
    printf("mapped memory contents (hex):\n");
    for(i = 0; i < blah; i++)
    	printf("<%02X>  ", map[size+i]);
    printf("\n");

    int test = 0;
    for(i =0; i < blah; i++){
    	if(map[size+i] != buffer[i])
    		test = 1;
    }

    if(test)
    	printf("The data previously written to the hole does NOT remain.\n\n");
    else
    	printf("The data previously written to the hole remains.\n\n");
}

void f(){
	open_file(testfile);

	map = mmap(0, 8192, PROT_READ, MAP_SHARED, fd, 0);
	if(map == MAP_FAILED){
		perror("mmap failed");
		exit(1);
	}
	printf("Reading from byte 4000\n");
	printf("<%02X>\n", map[4000]);
	printf("Reading from byte 8000\n");
	printf("<%02x>\n", map[8000]);
}

int main(int argc, char* argv[]){

	if(argc < 2){
		fprintf(stderr, "Not enough arguments. Please enter (a-f) as the argument.\n");
		exit(1);
	}

	signal(SIGSEGV, sig_handler);
    signal(SIGBUS, sig_handler);
	char c = *argv[1];
	switch(c){
        case('a'):
            printf("part a:\n");
            a();
            break;
        case('b'):
            printf("part b:\n");
            bc(1);
            break;
        case('c'):
            printf("part c:\n");
            bc(0);
            break;
        case('d'):
            printf("part d:\n");
            d();
            break;
        case('e'):
            printf("part e:\n");
            e();
            break;
        case('f'):
            printf("part f:\n");
            f();
            break;
        default:
            printf("Invalid Argument.  Please enter (a-f) as the argument\n");
            return 0;
    }
    return 0;

}