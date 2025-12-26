
#include"logging.h"

LoggingFile *logCreateLogger(FILE *fptr,LoggingLevel level){
	LoggingFile *out = malloc(sizeof(LoggingFile));
	out->level = level;
	out->fptr = fptr;
	return out;
}
void logRemoveLogger(LoggingFile *log){
	if(log->fptr <= stdin){}
	else if(log->fptr <= stderr){}
	else if(log->fptr <= stdout){}
	else{
		fclose(log->fptr);
	}
	free(log);
}


