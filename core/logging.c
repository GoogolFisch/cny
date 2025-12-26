
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

// will log to stdout if logger is NULL
// or respect the log level in the logger struct
// level isn't currently used?
#define LOG_WRITE(logger,level,...) if(logger == NULL){ \
		if(level <= LOG_ERROR) \
			fputs(LogLevelString[level]); \
		printf(__VA_ARGS__); \
	}else if(level >= logger->level){ \
		if(level <= LOG_ERROR) \
			fputs(LogLevelString[level]); \
		fprintf(logger->fptr,__VA_ARGS__); \
	}

