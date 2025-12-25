
#include"scope.h"



// creating these Objects!
ScopeObject *scopeMakeObject(){
	ScopeObject *out = malloc(sizeof(ScopeObject));
	out->next = NULL;
	out->wayNext = NULL;
	out->prev = NULL;
	out->length = 0;
	out->count = 1;
	out->lowerBound = -1;
	return out;
}
ScopeArray *scopeMakeArray(){
	ScopeArray *out = malloc(sizeof(ScopeArray));
	out->next = NULL;
	out->length = 0;
	out->count = 1;
	return out;
}
ScopeAnyArray *scopeMakeAnyArray(){
	ScopeAnyArray *out = malloc(sizeof(ScopeAnyArray));
	out->next = NULL;
	out->length = 0;
	out->count = 1;
	return out;
}
ScopeString *scopeMakeString(){
	ScopeString *out = malloc(sizeof(ScopeString));
	out->string = NULL;
	out->length = 0;
	out->count = 1;
	out->capacity = 0;
	return out;
}

// hashing string
int32_t scopeHashString(ScopeString *str){
	int32_t hash = 0;
	for(int i = 0;i < str->length;i++){
		hash += str->string[i];
		hash ^= hash >> 5;
		hash ^= hash << 17;
		hash += hash >> 12;
	}
	return hash;
}

// adding stuff into this
int32_t scopeInsertObject(
		ScopeObject *scope,
		ScopeValue typ,
		void *value,
		ScopeString *str
){
	uint32_t hash = scopeHashString(str);
	ScopeObject *current = scope;

	// step through the obect
	while(current->lowerBound > hash){
		if(current->next == NULL)
			break;
		current = current->next;
	}
	if(current->lowerBound >= hash){
		current = current->prev;
	}
	if(current->length + 1 >= SCOPE_CAPACITY){
		ScopeObject *create = scopeMakeObject();
		create->prev = current;
		create->next = current->next;
		current->next->prev = create;
		current->next = create;
		// splitting??
		int32_t splice = 0;
		uint32_t nHash = scopeHashString(create->content[create->length / 2].name);
		create->lowerBound = nHash;
		for(int32_t idx = current->length / 2;idx < current->length;idx++){
			create->content[splice] = current->content[idx];
			splice++;
		}
		create->length = splice;
		current->length -= splice;
		if(nHash <= hash){
			current = create;
		}
	}
	uint32_t cmpHash;
	int32_t idx;
	for(idx = 0;idx < current->length;idx++){
		cmpHash = scopeHashString(current->content[idx].name);
		if(cmpHash < hash)
			continue;
		break;
	}
	if(idx == 0){
		current->lowerBound = hash;
	}
	for(int32_t bind = current->length;bind > idx;bind--){
		current->content[bind] = current->content[bind - 1];
	}
	(*(int32_t*)value)++;
	current->content[idx].typ = typ;
	current->content[idx].name = str;
	current->content[idx].value = value;
	return 0;
}

int32_t scopeAppendArray(){
	return 0;
}

// 

