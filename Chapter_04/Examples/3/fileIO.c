#include <stdio.h>
#include <fcntl.h>

int main()
{
	int c;

	int rfd;

	rfd=open("myfile.txt",O_WRONLY|O_CREAT|O_TRUNC);
	
	fprintf(stderr,"My PID=%d\n", getpid());
	
	c=getchar();

}
