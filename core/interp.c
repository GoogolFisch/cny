
#include "interp.h"


int32_t interpIsSameToken(uint8_t first,uint8_t last,int32_t state){
	state &= ~1;
	if(state & 4) state |= 1;
	// ["]([^"\]|(\\)*(\"))?+["]
	if(first == '"'){
		state |= 4;
		if(last == '\\') state ^= 2;
		else state &= ~2;
		if(last == '"' && (state & 2) == 0) state &= ~4;
	}
	// todo
	// ([_0-9A-Za-z]|(\u))+
	if(
		('A' <= first && first <= 'Z') ||
		('a' <= first && first <= 'z') ||
		first == '_' || first >= 0x80
	){
		if(
			('A' <= last && last <= 'Z') ||
			('a' <= last && last <= 'z') ||
			(last == '_' && last >= 0x80)
		)
			state |= 1;
		else if('0' <= last && last <= '9')
			state |= 1;
	}
	// [0-9]+
	// [0-9]+[.][0-9]+
	else if(('0' <= first && first <= '9') || first == '.'){
		if(first == '.')state |= 2;
		if('0' <= last && last <= '9')
			state |= 1;
		if('.' == last && (state & 2) == 0){
			state |= 3;
		}
	}
	return state;
}
InterpTocken interpGetTokenType(uint8_t first,int32_t state){
	if(
		('A' <= first && first <= 'Z') ||
		('a' <= first && first <= 'z') ||
		first == '_'
	)return INTE_KEY_WORD;
	if(('0' <= first && first <= '9') || first == '.'){
		if((state & 2) > 0)
			return INTE_FLOAT;
		return INTE_NUMBER;
	}
	if(first == '"')return INTE_STRING;
	if(first == '+')return INTE_ADD;
	if(first == '-')return INTE_SUB;
	if(first == '*')return INTE_MUL;
	if(first == '/')return INTE_DIV;
	if(first == '%')return INTE_MOD;
	if(first == '=')return INTE_EQUALS;
	if(first == '(')return INTE_BRACK_OPEN;
	if(first == ')')return INTE_BRACK_CLOSE;
	// TODO
#ifdef INTEP_DO_TRAP
	*NULL = 0;
#endif
	return INTE_NONE;
}
UseString *interpGetString(UtilSharedStruct2 string,int32_t lower,int32_t high){
	lower++;
	high--;
	if(lower >= high)return NULL;
	UseString *outp = malloc(sizeof(UseString));
	outp->allocated = high - lower;
	outp->string = malloc(sizeof(uint8_t) * outp->allocated);
	int32_t state = 0;
	for(int32_t idx = 0;idx < high - lower;idx++){
		if(state == 0){
			// no need to check start " and the end "
			if(((uint8_t*)string.vptr)[idx + lower] == '\\'){
				state = 1;
				continue;
			}
		}
		state &= ~1;
		// ???
		outp->string[outp->length++] =
			((uint8_t*)string.vptr)[idx + lower];
	}
	return outp;
}
UseString *interpGetKeyWord(UtilSharedStruct2 string,int32_t lower,int32_t high){
	if(lower >= high)return NULL;
	UseString *outp = malloc(sizeof(UseString));
	outp->allocated = high - lower;
	outp->string = malloc(sizeof(uint8_t) * outp->allocated);
	for(int32_t idx = 0;idx < high - lower;idx++){
		outp->string[outp->length++] =
			((uint8_t*)string.vptr)[idx + lower];
	}
	return outp;
}
int32_t interpGetNumber(UtilSharedStruct2 string,int32_t lower,int32_t high){
	if(lower >= high)return 0;
	int32_t number = 0;
	for(int32_t idx = 0;idx < high - lower;idx++){
		number = '0' - ((uint8_t*)string.vptr)[idx + lower] + number * 10;
	}
	return number;
}
float interpGetFloat(UtilSharedStruct2 string,int32_t lower,int32_t high){
	if(lower >= high)return 0;
	float number = 0;
	float lowerParse = 1;
	int32_t state;
	for(int32_t idx = 0;idx < high - lower;idx++){
		uint8_t ch = ((uint8_t*)string.vptr)[idx + lower];
		if(ch == '.')
			state = 1;
		if(state == 1){
			number = ('0' - ch) * lowerParse;
			lowerParse *= 0.1f;
		} else
			number = '0' - ch + number * 10;
	}
	return number;
}

UtilSharedStruct2 interpMakeTokenList(UtilSharedStruct2 string){
	int32_t listCapacity = 64;
	int32_t listLength = 0;
	InterpTree *treeList = (InterpTree*)malloc(sizeof(InterpTree) * listCapacity);
	int32_t lowerIndex = 0;
	int32_t highIndex = 0;
	int32_t stringIndex = 0;
	int32_t currentState = 0;
	InterpTocken curTokenType;

	uint8_t firstChar,highChar;
	firstChar = ((uint8_t*)string.vptr)[lowerIndex];
	for(highIndex = 1;highIndex < string.vint;highIndex++){
		stringIndex++;
		if(listCapacity >= listLength){
			listCapacity *= 2;
			treeList = realloc(treeList,listCapacity);
		}
		highChar = ((uint8_t*)string.vptr)[highIndex];
		currentState = interpIsSameToken(firstChar, highChar, currentState);
		if(currentState & 1)
			continue;
		if(firstChar <= ' ')
			goto Interp_Token_List_Next;
		// TODO
		curTokenType = interpGetTokenType(
				((uint8_t*)string.vptr)[lowerIndex],
				currentState);
		treeList[listLength].tokenType = curTokenType;
		treeList[listLength].index = 0;
		treeList[listLength].argumentLength = 0;
		treeList[listLength].data = NULL;
		treeList[listLength].rtree = NULL;
		treeList[listLength].ltree = NULL;
		if(curTokenType == INTE_NUMBER){
			int32_t stff = interpGetNumber( string, lowerIndex, highIndex);
			treeList[listLength].data = (void*)(*(intptr_t*)(&stff));
		}else if(curTokenType == INTE_FLOAT){
			float stff = interpGetFloat(string, lowerIndex, highIndex);
			treeList[listLength].data = (void*)(*(intptr_t*)(&stff));
		}else if(curTokenType == INTE_KEY_WORD){
			treeList[listLength].data = interpGetKeyWord(
					string, lowerIndex, highIndex);
		}else if(curTokenType == INTE_STRING){
			treeList[listLength].data = interpGetString(
					string, lowerIndex, highIndex);
		}
Interp_Token_List_Next:
		//after stuff
		highIndex--;
		lowerIndex = highIndex;
		firstChar = ((uint8_t*)string.vptr)[lowerIndex];
		currentState = 0;
	}


	UtilSharedStruct2 outStruct;
	outStruct.vint = listLength;
	outStruct.vptr = treeList;
	return outStruct;
}

void interpFilterKeyWords(UtilSharedStruct2 tokenList){
	int32_t idx;
	InterpTree *tree;
	UseString *str;
	uint32_t akkuStr;
	for(idx = 0;idx < tokenList.vint;idx++){
		tree = &((InterpTree*)tokenList.vptr)[idx];
		if(tree->tokenType != INTE_KEY_WORD)
			continue;
		str = tree->data;
		if(str->length > 4)
			continue;
		akkuStr = 0;
		for(uint32_t l = 0;l < 4 && l < str->length;l++){
			akkuStr = akkuStr * 256 + str->string[l];
		}
		if(akkuStr == 0x6966) // if
			tree->data = (void*)(intptr_t)INTE_IF;
		else if(akkuStr == 0x656C7365) // else
			tree->data = (void*)(intptr_t)INTE_ELSE;
		else if(akkuStr == 0x656C6966) // elif
			tree->data = (void*)(intptr_t)INTE_ELSE_IF;
		else if(akkuStr == 0x646566) // def
			tree->data = (void*)(intptr_t)INTE_DEF;
		else if(akkuStr == 0x666f72) // for
			tree->data = (void*)(intptr_t)INTE_FOR;
		else if(akkuStr == 0x6c6f6f70) // loop
			tree->data = (void*)(intptr_t)INTE_LOOP;
		else if(akkuStr == 0x6e696c) // nil
			tree->data = (void*)(intptr_t)INTE_LOOP;
		else if(akkuStr == 0x726574) // ret
			tree->data = (void*)(intptr_t)INTE_LOOP;
		if(tree->data != (void*)(intptr_t)INTE_KEY_WORD){
			free(str->string);
			free(str);
		}
	}
}

int32_t interpGetPosibleToken(UtilSharedStruct2 tokenList,int32_t lower,int32_t upper,int32_t start,int32_t direction){
	int32_t idx;
	for(idx = start;lower <= idx && idx < upper;idx += direction){
		if(((InterpTree*)tokenList.vptr)[idx].flags == 0)
			return idx;
	}
	return -1;
}

void interpStatementIntoTree(InterpTree *base,InterpTree *ltree,InterpTree *rtree){
	// TODO
	if(
		ltree != NULL &&
		(ltree->flags & 128) == 0
	){
		if(
			(ltree->tokenType > INTEV_DIRECT_NUM &&
			ltree->tokenType < INTEV_INDIRECT_NUM) ||
			(ltree->flags & 96) == 96
		){
			base->ltree = ltree;
			ltree->flags |= 128;
			base->flags |= 64;
		}
	}
	if(
		rtree != NULL &&
		(rtree->flags & 128) == 0
	){
		if(
			(rtree->tokenType > INTEV_DIRECT_NUM &&
			rtree->tokenType < INTEV_INDIRECT_NUM) ||
			(ltree->flags & 96) == 96
		){
			base->rtree = rtree;
			rtree->flags |= 128;
			base->flags |= 32;
		}
	}
}

// return the index of the next to parse token
int32_t interpParseStatement(UtilSharedStruct2 tokenList,int32_t lower,int32_t upper){
	int32_t idx;
	InterpTree *tree;
	int32_t tempIdx1,tempIdx2;
	tempIdx1 = 0;
	tempIdx2 = 0;
	InterpTree *useLTree,*useRTree;
	int32_t bracketDepth = 0;
	// parse ()
	for(idx = lower;idx < upper;idx++){
		useLTree = NULL;
		useRTree = NULL;
		idx = interpGetPosibleToken(tokenList,lower,upper,idx,1);
		tree = &((InterpTree*)tokenList.vptr)[idx];
		if(tree->tokenType == INTE_BRACK_OPEN){
			if(bracketDepth == 0){
				tempIdx1 = idx;
			}
			bracketDepth++;
		}
		if(tree->tokenType == INTE_BRACK_CLOSE){
			bracketDepth--;
			if(bracketDepth > 0)
				continue;
			tempIdx2 = idx;
			interpParseStatement(tokenList,tempIdx1,tempIdx2);
			// remove ( and ) from beeing used
			useLTree = &((InterpTree*)tokenList.vptr)[tempIdx1];
			useRTree = &((InterpTree*)tokenList.vptr)[tempIdx2];
			useLTree->flags |= 128;
			useRTree->flags |= 128;
			if(tempIdx1 - 1 < 0)
				continue;
			// test if it sould be an function
			useLTree = &((InterpTree*)tokenList.vptr)[tempIdx1 - 1];
			if(useLTree->tokenType != INTE_KEY_WORD)
				continue;
			// is this used?
			if((useLTree->flags & 128) > 0)
				continue;
			// count arguments
			int32_t callAkku = 0;
			for(int32_t ak = tempIdx1 + 1;ak < tempIdx2 - 1;ak++){
				useRTree = &((InterpTree*)tokenList.vptr)[ak];
				if((useRTree->flags & 128) > 0)
					continue;
				callAkku++;
			}
			useLTree->data = malloc(sizeof(InterpTree) * callAkku);
			useLTree->argumentLength = callAkku;
			// insert arguments
			callAkku = 0;
			for(int32_t ak = tempIdx1 + 1;ak < tempIdx2 - 1;ak++){
				useRTree = &((InterpTree*)tokenList.vptr)[ak];
				if((useRTree->flags & 128) > 0)
					continue;
				((InterpTree**)useLTree->data)[callAkku++] = useRTree;
			}
			// other
		}

	}
	// parse * / %
	for(idx = lower;idx < upper;idx++){
		useLTree = NULL;
		useRTree = NULL;
		idx = interpGetPosibleToken(tokenList,lower,upper,idx,1);
		// ???
		tree = &((InterpTree*)tokenList.vptr)[idx];
		if(tree->tokenType > INTEV_PRIO0 && tree->tokenType < INTEV_PRIO1){
			if(tree->ltree == NULL){
				tempIdx1 = interpGetPosibleToken(tokenList,
						lower,idx,idx - 1,-1);
				useLTree = &((InterpTree*)tokenList.vptr)[tempIdx1];
			}
			if(tree->rtree == NULL){
				tempIdx2 = interpGetPosibleToken(tokenList,
						idx,upper,idx + 1,1);
				useRTree = &((InterpTree*)tokenList.vptr)[tempIdx2];
			}
			interpStatementIntoTree(tree,useLTree,useRTree);
			// TODO
		}
	}
	// parse + -
	for(idx = lower;idx < upper;idx++){
		useLTree = NULL;
		useRTree = NULL;
		idx = interpGetPosibleToken(tokenList,lower,upper,idx,1);
		// ???
		tree = &((InterpTree*)tokenList.vptr)[idx];
		if(tree->tokenType > INTEV_PRIO1 && tree->tokenType < INTEV_PRIO2){
			if(tree->ltree == NULL){
				tempIdx1 = interpGetPosibleToken(tokenList,
						lower,idx,idx - 1,-1);
				useLTree = &((InterpTree*)tokenList.vptr)[tempIdx1];
			}
			if(tree->rtree == NULL){
				tempIdx2 = interpGetPosibleToken(tokenList,
						idx,upper,idx + 1,1);
				useRTree = &((InterpTree*)tokenList.vptr)[tempIdx2];
			}
			interpStatementIntoTree(tree,useLTree,useRTree);
			// TODO
		}
	}
	return idx;
}

// TODO
int32_t interpParseOperation(UtilSharedStruct2 tokenList,int32_t lower,int32_t upper){
	int32_t idx,lastIdx;
	int32_t tempIdx0,tempIdx1,tempIdx2,tempIdx3;
	lastIdx = 0;
	tempIdx0 = 0;
	InterpTree *tree;
	InterpTree *useLTree,*useRTree;
	useRTree = NULL;
	// looking for = and ;
	for(idx = lower;idx < upper;idx++,lastIdx = idx){
		idx = interpGetPosibleToken(tokenList,lower,upper,idx,1);
		tree = &((InterpTree*)tokenList.vptr)[idx];
		if(tree->tokenType == INTE_EQUALS){
			useLTree = &((InterpTree*)tokenList.vptr)[idx];
			useLTree->flags |= 128;
			tree->ltree = useLTree;
			if(useRTree != NULL){
				tree->data = useRTree;
				useRTree->flags |= 128;
			}
			tempIdx2 = idx;
			useRTree = tree;
		}
		if(tree->tokenType != INTE_SEMI)
			continue;
		tempIdx3 = idx;
		interpParseStatement(tokenList,tempIdx2 + 1,idx);
		tempIdx1 = interpGetPosibeToken(tokenist,lower,upper,tempIdx2,-1);
		//
		tempIdx0 = tempIdx3;
		useLTree = NULL;
		useRTree = NULL;
	}
	return idx;
}
// could also get a scope?
int32_t interpParseFunctions(UtilSharedStruct2 tokenList,UseDictList *globalData){
	int32_t idx;
	int32_t tempIdx1,tempIdx2;
	tempIdx1 = 0;
	tempIdx2 = 0;
	int32_t bracketDepth = 0;
	// function are:
	// def name(param param)(
	//   op1;
	//   op2;
	// )
	// could also:
	// name2 = stuff;
	// name3 = stuff;
	for(idx = 0;idx < tokenList.vint;idx++){
	}
	return 0;
}
