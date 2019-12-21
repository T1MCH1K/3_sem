#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

char* data = NULL;
int cbit = 0;
int totsize = 0;
int cpid, ppid;

int buffsize = 8;

char* input = "input.c";
char* output = "output.c";

char mask() {
	    return 1 << (cbit % 8);
}
void expand(int d) {
       	static int datasize = 0;
        if(datasize == 0) {
	        data = malloc(1);
	        datasize = 1;				  
      	}
	while(d < datasize)
        data = realloc(data, datasize *= 2);
}
void setzero(int n) {
	expand(cbit / 8 + 1);
        data[cbit / 8] &= ~mask();
        cbit++;
        kill(ppid, SIGUSR1);
}
void setone(int n) {
	    expand(cbit / 8 + 1);
            data[cbit / 8] |= mask();
	    cbit++;
     	    kill(ppid, SIGUSR1);
}
void finish(int n) {
	    int fd = open(output, O_CREAT | O_WRONLY, 0600);
            write(fd, data, cbit / 8);
	    exit(0);
}
void sendbit(int n) {
	    if(cbit / 8 == totsize)
  	          return;
            if(data[cbit / 8] & mask()) {
    	        cbit++;
	        kill(cpid, SIGUSR2);
	    }
	    else {
	            cbit++;
	            kill(cpid, SIGUSR1);
	    }
}
int main() {
	signal(SIGUSR1, setzero);
        signal(SIGUSR2, setone);
        signal(SIGINT, finish);
        ppid = getpid();
	cpid = fork();
	if(cpid) {
	        signal(SIGUSR1, sendbit);
	        signal(SIGINT, SIG_DFL);
	        int fd = open(input, O_RDONLY);
	        expand(buffsize);
	        for(int d; d = read(fd, &data[totsize], buffsize); totsize += d)
			expand(totsize + buffsize);
	        write(STDOUT_FILENO, data, totsize);
		sendbit(0);
		while(cbit / 8 != totsize)
	            usleep(1);
	        kill(cpid, SIGINT);											    
	}
	else
        	while(1) pause();
}
