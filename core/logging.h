
#ifndef LOGGING_H_
#define LOGGING_H_

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

typedef enum LoggingLevel{
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARNING,
	LOG_CRITICAL,
	LOG_ERROR,
}LoggingLevel;

typedef struct LoggingFile{
	FILE *fptr;
	LoggingLevel level;
}LoggingFile;

char *LogLevelString[] = {
	"[DEBUG]",
	"[INFO]",
	"[WARNING]",
	"[CRITICAL]",
	"[ERROR]",
};

//
LoggingFile *logCreateLogger(FILE *fptr,LoggingLevel level);
/*{
	LoggingFile *out = malloc(sizeof(LoggingFile));
	out->level = level;
	out->fptr = fptr;
	return out;
} // */
void logRemoveLogger(LoggingFile *log);
/*{
	if(log->fptr <= stdin){}
	else if(log->fptr <= stderr){}
	else{
		close(log->fptr);
	}
	free(log);
} // */
void logWrite(LoggingFile* logger,LoggingLevel level,char *msg, ...);

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

#endif
