// gcc malloc_.c -o malloc_.o

#include<stdio.h>
#include<stdlib.h>

#define FLL 64

int main(int argc,char **argv){
	void *filling[FLL] = {};
	size_t sz;
	for(int i = 0;i < FLL;i++){
		sz = 4 + (i & 3);
		filling[i] = malloc(sizeof(char) * (sz << (i >> 2)));
		printf("%p\n",filling[i]);
	}
	for(int i = 0;i < FLL;i++){
		free(filling[i]);
	}
}
