
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

// copy stuff
ScopeString *scopeCopyString(ScopeString *str){
	if(str == NULL)return NULL;
	ScopeString *out = malloc(sizeof(ScopeString));
	out->string = malloc(sizeof(char) * str->capacity);
	for(int i = 0;i < str->length;i++){
		out->string[i] = str->string[i];
	}
	out->length = str->length;
	str->capacity = str->capacity;
	out->count = 1;
	return out;
}
ScopeArray *scopeCopyArray(ScopeArray *arr){
	if(arr == NULL)return NULL;
	ScopeArray *out = malloc(sizeof(ScopeArray));
	ScopeArray *current = out;
	ScopeArray *cpFrom = arr;
	while(1){
		current->count = 1;
		current->typ = cpFrom->typ;
		current->length = cpFrom->length;
		for(int i = 0;i < cpFrom->length;i++){
			current->content[i].value = cpFrom->content[i].value;
		}
		cpFrom = cpFrom->next;
		if(cpFrom == NULL){
			current->next = NULL;
			continue;
		}
		current->next = malloc(sizeof(ScopeArray));
		current = current->next;
	}
	return out;
}
ScopeAnyArray *scopeCopyAnyArray(ScopeAnyArray *arr){
	if(arr == NULL)return NULL;
	ScopeAnyArray *out = malloc(sizeof(ScopeAnyArray));
	ScopeAnyArray *current = out;
	ScopeAnyArray *cpFrom = arr;
	while(1){
		current->count = 1;
		current->length = cpFrom->length;
		for(int i = 0;i < cpFrom->length;i++){
			current->content[i].typ = cpFrom->content[i].typ;
			current->content[i].value = cpFrom->content[i].value;
		}
		cpFrom = cpFrom->next;
		if(cpFrom == NULL){
			current->next = NULL;
			continue;
		}
		current->next = malloc(sizeof(ScopeAnyArray));
		current = current->next;
	}
	return out;
}
ScopeObject *scopeCopyObject(ScopeObject *arr){
	if(arr == NULL)return NULL;
	ScopeObject *out = malloc(sizeof(ScopeObject));
	ScopeObject *current = out;
	ScopeObject *cpFrom = arr;
	current->prev = NULL;
	while(1){
		current->count = 1;
		current->length = cpFrom->length;
		for(int i = 0;i < cpFrom->length;i++){
			current->content[i].typ = cpFrom->content[i].typ;
			current->content[i].name = cpFrom->content[i].name;
			current->content[i].value = cpFrom->content[i].value;
		}
		cpFrom = cpFrom->next;
		if(cpFrom == NULL){
			current->next = NULL;
			continue;
		}
		current->next = malloc(sizeof(ScopeObject));
		current->wayNext = current->next;
		current->next->prev = current;
	}
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

int32_t scopeAppendArray(
		ScopeArray *scope,
		ScopeValue typ,
		void *value
		){
	ScopeArray *current = scope;
	// if there is anything (here even potentially)
	// regect if it doesn't match the type!
	if(current->length > 0 && current->next != NULL && current->typ != typ)
		// yield error for not fitting in!
		return 1;
	// used 4 when this is empty!
	current->typ = typ;
	while(current->next != NULL)
		current = current->next;
	if(current->length >= SCOPE_CAPACITY){
		current->next = malloc(sizeof(ScopeArray));
		// isn't very usefull...
		current->next->typ = current->typ;
		current = current->next;
		current->length = 0;
		current->next = NULL;
	}
	current->content[current->length++].value = value;
	return 0;
}
int32_t scopeAppendAnyArray(
		ScopeAnyArray *scope,
		ScopeValue typ,
		void *value
		){
	ScopeAnyArray *current = scope;
	while(current->next != NULL)
		current = current->next;
	if(current->length >= SCOPE_CAPACITY){
		current->next = malloc(sizeof(ScopeAnyArray));
		// isn't very usefull...
		current = current->next;
		current->length = 0;
		current->next = NULL;
	}
	current->content[current->length++].value = value;
	current->content[current->length++].typ = typ;
	return 0;
}
int32_t scopeInsertArray(
		ScopeArray *scope,
		ScopeValue typ,
		void *value,
		int32_t position
		){
	ScopeArray *current = scope;
	int32_t counted = 0;
	// if there is anything (here even potentially)
	// regect if it doesn't match the type!
	if(current->length > 0 && current->next != NULL && current->typ != typ)
		// yield error for not fitting in!
		return 1;
	// used 4 when this is empty!
	current->typ = typ;
	while(current->next != NULL){
		counted += current->length;
		if(counted > position)
			break;
		if(counted == position){
			// edge case?
			if(current->next->length < SCOPE_CAPACITY){
				current = current->next;
				counted += current->length;
			}
			break;
		}
		current = current->next;
	}
	counted -= current->length;
	if(current->length >= SCOPE_CAPACITY){
		ScopeArray *create = malloc(sizeof(ScopeArray));
		create->next = current->next;
		create->count = 1;
		create->typ = current->typ;
		current->next = create;
		int32_t splice = 0;
		for(int32_t idx = current->length / 2;idx < current->length;idx++){
			create->content[splice] = current->content[idx];
			splice++;
		}
		counted += splice;
		current->length -= splice;
		create->length = splice;
		if(counted >= position)
			counted -= splice;
		else current = create;
	}
	// TODO make insert!
	for(int idx = current->length;idx > 0;idx--){
		current->content[idx] = current->content[idx - 1];
	}
	current->content[position - counted].value = value;
	current->length++;
	return 0;
}
int32_t scopeInsertAnyArray(
		ScopeAnyArray *scope,
		ScopeValue typ,
		void *value,
		int32_t position
		){
	ScopeAnyArray *current = scope;
	int32_t counted = 0;
	while(current->next != NULL){
		counted += current->length;
		if(counted > position)
			break;
		if(counted == position){
			// edge case?
			if(current->next->length < SCOPE_CAPACITY){
				current = current->next;
				counted += current->length;
			}
			break;
		}
		current = current->next;
	}
	counted -= current->length;
	if(current->length >= SCOPE_CAPACITY){
		ScopeAnyArray *create = malloc(sizeof(ScopeAnyArray));
		create->next = current->next;
		create->count = 1;
		current->next = create;
		int32_t splice = 0;
		for(int32_t idx = current->length / 2;idx < current->length;idx++){
			create->content[splice] = current->content[idx];
			splice++;
		}
		counted += splice;
		current->length -= splice;
		create->length = splice;
		if(counted >= position)
			counted -= splice;
		else current = create;
	}
	// TODO make insert!
	for(int idx = current->length;idx > 0;idx--){
		current->content[idx] = current->content[idx - 1];
	}
	current->content[position - counted].value = value;
	current->content[position - counted].typ = typ;
	current->length++;
	return 0;
}

// 

