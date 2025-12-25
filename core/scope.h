
#ifndef SCOPE_H_
#define SCOPE_H_

#include<stdint.h>
#include<stdlib.h>
#include<stdio.h>

typedef enum ScopeValue{
	SCOPE_NONE,
	SCOPE_NUMBER,
	SCOPE_FLOAT,
	SCOPE_STRING,
	SCOPE_ARRAY,
	SCOPE_ANY_ARRAY,
	SCOPE_FUNCTION,
	SCOPE_OBJECT,
}ScopeValue;

/*typedef struct Scope{
	ScopeValue valType;
	union{
		int32_t ival;
		float fval;
		char *strVal;
		void *objVal;
	};
	int32_t length;
	int32_t capacity;
}Scope;*/
typedef union ScopeCombination{
	int32_t ival;
	float fval;
	char *strVal;
	void *objVal;
}ScopeCombination;

#define SCOPE_CAPACITY 64

typedef struct ScopeString{
	int32_t count;
	int32_t length;
	int32_t capacity;
	char *string;
}ScopeString;

typedef struct ScopeArray{
	int32_t count;
	ScopeValue typ;
	int32_t length;
	struct ScopeArray *next;
	struct {
		void *value;
		// untion ScopeCombination value;
	}content[SCOPE_CAPACITY];
}ScopeArray;

typedef struct ScopeAnyArray{
	int32_t count;
	int32_t length;
	struct ScopeAnyArray *next;
	struct {
		ScopeValue typ;
		void *value;
		// untion ScopeCombination value;
	}content[SCOPE_CAPACITY];
}ScopeAnyArray;

typedef struct ScopeObject{
	int32_t count;
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



// creating these Objects!
ScopeObject *scopeMakeObject();
ScopeArray *scopeMakeArray();
ScopeAnyArray *scopeMakeAnyArray();
ScopeString *scopeMakeString();
// hashing string
int32_t scopeHashString(ScopeString *str);
// adding stuff into this
int32_t scopeInsertObject(
		ScopeObject *scope,
		ScopeValue typ,
		void *value,
		ScopeString *str
);

#endif
