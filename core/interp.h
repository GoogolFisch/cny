
#ifndef INTERP_H_
#define INTERP_H_
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include"../util.h"
#include"../structures.c"


typedef enum InterpTocken{
	INTE_NONE,
	INTEV_DIRECT_NUM, //
	INTE_NUMBER,
	INTE_FLOAT,
	INTE_KEY_WORD,
	INTE_STRING,
	INTEV_INDIRECT_NUM, //
	INTEV_PRIO0, //
	INTE_MUL,
	INTE_DIV,
	INTE_MOD,
	INTEV_PRIO1, //
	INTE_ADD,
	INTE_SUB,
	INTEV_PRIO2, //
	INTEV_PRIO3, //
	INTE_EQUALS,
	INTE_BRACK_OPEN,
	INTE_BRACK_CLOSE,
	INTE_SEMI,
	INTEV_WORD, //
	INTE_IF,
	INTE_ELSE,
	INTE_ELSE_IF,
	INTE_DEF,
	INTE_FOR,
	INTE_LOOP,
	INTE_NIL,
	INTE_RET,

	INTEV_END, //
}InterpTocken;

typedef struct InterpTree{
	InterpTocken tokenType;
	void *data;
	int32_t argumentLength;
	int32_t flags;
	struct InterpTree *ltree;
	struct InterpTree *rtree;
	int32_t index;
}InterpTree;


// convert the input-string into program
UtilSharedStruct2 interpMakeTokenList(UtilSharedStruct2 string);
// turn if -> INTE_IF
void interpFilterKeyWords(UtilSharedStruct2 list);
// this could parse stuff with idk
int32_t interpParseStatement(UtilSharedStruct2 tokenList,int32_t lower,int32_t upper);
// this could parse stuff with idk
int32_t interpParseOperation(UtilSharedStruct2 tokenList,int32_t lower,int32_t upper);
// only top level
int32_t interpParseFunctions(UtilSharedStruct2 tokenList,UseDictList *globalData);


#endif
