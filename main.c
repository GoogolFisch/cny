#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define BUFFER_LENGTH (1024*1024)
//#define STDOUT_FILENO STDOUT

void startColor(int col){
	printf("\x1b[%dm",col);
}
void moveCursor(int x,int y){
	printf("\x1b[%d;%dH",y,x);
}
void eraseEntireScreen(void){
	printf("\x1b[2J");
}

int main( int argc, char *argv[] ) {
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	eraseEntireScreen();

	startColor(34);
	printf("lines %d\n",w.ws_row);
	printf("columns %d\n",w.ws_col);
	startColor(0);
	moveCursor(0,0);
	printf("lines %d\n",w.ws_row);
	printf("columns %d\n",w.ws_col);

	return 0;
}

