#ifndef STRUCTURES_C_
#define STRUCUTRES_C_
#include<stdint.h>
#include<stdlib.h>

typedef struct UseString{
	uint32_t length;
	uint32_t allocated;
	char *string;
} UseString;
#define USE_DICT_LIST_COUNT 64
typedef struct UseDictList{
	struct{
		UseString key;
		UseString value;
	} kvl[USE_DICT_LIST_COUNT];
	int32_t length;
	struct UseDictList *next;
	struct UseDictList *farNext;
	struct UseDictList *prev;
} UseDictList;


UseString *useMakeString(int32_t length){
	UseString *out = (UseString*)malloc(sizeof(UseString));
	out->length = length;
	out->allocated = 1 << (sizeof(int32_t) * 8 - __builtin_clz(length));
	out->string = malloc(sizeof(char) * out->allocated);
	return out;
}

#undef USE_DICT_LIST_COUNT
// 64
#endif
