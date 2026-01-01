
#ifndef SCOPE_H_
#define SCOPE_H_

#include<stdint.h>
#include<stdlib.h>
#include<stdio.h>
#include"../util.h"

typedef enum ScopeValue{
	SCOPE_NONE,
	SCOPE_NUMBER,
	SCOPE_FLOAT,
	SCOPE_STRING,
	SCOPE_ARRAY,
	SCOPE_ANY_ARRAY,
	SCOPE_FUNCTION,
	SCOPE_BUILD_IN_FUNCTION,
	SCOPE_OBJECT,
}ScopeValue;

#define SCOPE_CAPACITY 64

typedef struct ScopeString{
	int32_t count; // count of instances!
	int32_t length;
	int32_t capacity;
	char *string;
}ScopeString;

typedef struct ScopeArray{
	int32_t count; // count of instances!
	ScopeValue typ;
	int32_t length;
	struct ScopeArray *next;
	struct {
		void *value;
		// untion ScopeCombination value;
	}content[SCOPE_CAPACITY];
}ScopeArray;

typedef struct ScopeAnyArray{
	int32_t count; // count of instances!
	int32_t length;
	struct ScopeAnyArray *next;
	struct {
		ScopeValue typ;
		void *value;
		// untion ScopeCombination value;
	}content[SCOPE_CAPACITY];
}ScopeAnyArray;

typedef struct ScopeObject{
	int32_t count; // count of instances!
	uint32_t lowerBound;
	struct ScopeObject *next;
	struct ScopeObject *wayNext;
	struct ScopeObject *prev;
	int32_t length;
	struct {
		ScopeValue typ;
		ScopeString *name;
		void *value;
		// untion ScopeCombination value;
	}content[SCOPE_CAPACITY];
}ScopeObject;



// === creating these Objects!
ScopeObject *scopeMakeObject();
ScopeArray *scopeMakeArray();
ScopeAnyArray *scopeMakeAnyArray();
ScopeString *scopeMakeString();
// === copy stuff
ScopeString *scopeCopyString(ScopeString *str);
ScopeArray *scopeCopyArray(ScopeArray *arr);
ScopeAnyArray *scopeCopyAnyArray(ScopeAnyArray *arr);
ScopeObject *scopeCopyObject(ScopeObject *arr);
// === scope counting stuff!
void scopeUp(void *value);
// will free if it should be freed
void scopeTst(void *value);
void scopeDown(void *value);
// === hashing string
int32_t scopeHashString(ScopeString *str);
//int32_t scopeHashString__(ScopeString *str);
//int scopeEqualString_(ScopeString *s1,ScopeString *s2,int32_t h2);
int scopeEqualString(ScopeString *s1,ScopeString *s2);
// === adding stuff into this
int32_t scopeInsertObject(
		ScopeObject *scope,
		ScopeValue typ,
		void *value,
		ScopeString *str
);
int32_t scopeAppendArray(
		ScopeArray *scope,
		ScopeValue typ,
		void *value
);
int32_t scopeAppendAnyArray(
		ScopeAnyArray *scope,
		ScopeValue typ,
		void *value
		);
int32_t scopeInsertArray(
		ScopeArray *scope,
		ScopeValue typ,
		void *value,
		int32_t position
);
int32_t scopeInsertAnyArray(
		ScopeAnyArray *scope,
		ScopeValue typ,
		void *value,
		int32_t position
);
// === override stuff in arrays
int32_t scopeReplaceArray(
		ScopeArray *scope,
		ScopeValue typ,
		void *value,
		int32_t position
);
int32_t scopeReplaceAnyArray(
		ScopeAnyArray *scope,
		ScopeValue typ,
		void *value,
		int32_t position
);
// === override stuff in arrays
UtilSharedStruct2 scopeRemoveObject(
		ScopeObject *scope,
		ScopeString *str
);
UtilSharedStruct2 scopePopArray(
		ScopeArray *scope
);
UtilSharedStruct2 scopePopAnyArray(
		ScopeAnyArray *scope
);
UtilSharedStruct2 scopeRemoveArray(
		ScopeArray *scope,
		int32_t position
);
UtilSharedStruct2 scopeRemoveAnyArray(
		ScopeAnyArray *scope,
		int32_t position
);
// === ...

#endif
