
#include "interp.h"


// TODO add comments!
int32_t interpIsSameToken(uint8_t first,uint8_t last,int32_t state){
	state &= ~1;
	if(state & 4) state |= 1;
	// ["]([^"\]|(\\)*(\"))?+["]
	if(first == '"'){
		state |= 4;
		if(last == '\n' && (state & 2) == 0)
			state &= ~6;
		else if(last == '\\') state ^= 2;
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
InterpToken interpGetTokenType(uint8_t first,int32_t state){
	if(
		('A' <= first && first <= 'Z') ||
		('a' <= first && first <= 'z') ||
		first == '_'
	)return INTE_KEY_WORD;
	if(('0' <= first && first <= '9') || first == '.'){
		if(state == 2)
			return INTE_DOT;
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
	if(first == ';')return INTE_SEMI;
	if(first == ';')return INTE_KOMMA;
	if(first == '(')return INTE_BRACK_OPEN;
	if(first == ')')return INTE_BRACK_CLOSE;
	if(first == '[')return INTE_SQ_BRACK_OPEN;
	if(first == ']')return INTE_SQ_BRACK_CLOSE;
	// TODO adding more token translations
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
		// check if escaped
		if((state & 1) == 0)
			outp->string[outp->length++] =
				((uint8_t*)string.vptr)[idx + lower];
		else{
			char ch = ((uint8_t*)string.vptr)[idx + lower];
			if(ch >= '0' && ch <= '9')
				outp->string[outp->length++] = ch & 0x1f;
			else if(ch >= 'A' && ch <= 'Z')
				outp->string[outp->length++] = ch & 0x1f;
			else if(ch >= 'a' && ch <= 'z')
				outp->string[outp->length++] = (ch & 0x1f) + 27;
			else
				outp->string[outp->length++] =
					((uint8_t*)string.vptr)[idx + lower];
		}
		state &= ~1;
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
	InterpToken curTokenType;

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

void interpFilter_CombineString(UtilSharedStruct2 tokenList,int32_t current){
	int32_t idx;
	InterpTree *tree;
	InterpTree *base = &((InterpTree*)tokenList.vptr)[current];
	UseString *str;
	str = (UseString*)(base->data);
	int32_t counted = str->length;
	for(idx = current + 1;idx < tokenList.vint;idx++){
		tree = &((InterpTree*)tokenList.vptr)[idx];
		if(tree->tokenType != INTE_STRING)
			break;
		str = (UseString*)(tree->data);
		counted += str->length;
		tree->flags |= 128;
	}
	UseString *strInto = malloc(sizeof(UseString));
	strInto->length = counted;
	strInto->allocated = counted;
	strInto->string = malloc(sizeof(char) * counted);
	counted = 0;
	for(idx = current;idx < tokenList.vint;idx++){
		tree = &((InterpTree*)tokenList.vptr)[idx];
		if(tree->tokenType != INTE_STRING)
			break;
		str = (UseString*)(tree->data);
		for(uint32_t cp = 0;cp < str->length;cp++){
			strInto->string[counted + cp] = str->string[cp];
		}
		counted += str->length;
	}
	str = (UseString*)(base->data);
	free(str->string);
	free(str);
	base->data = strInto;
}

void interpFilterKeyWords(UtilSharedStruct2 tokenList){
	int32_t idx;
	InterpTree *tree;
	UseString *str;
	uint32_t akkuStr;
	for(idx = 0;idx < tokenList.vint;idx++){
		tree = &((InterpTree*)tokenList.vptr)[idx];
		if((tree->flags & 128) > 0)
			// should ignore!
			continue;
		if(tree->tokenType == INTE_STRING)
			interpFilter_CombineString(tokenList,idx);
		if(tree->tokenType != INTE_KEY_WORD){
			continue;
		}
		str = tree->data;
		if(str->length > 4)
			continue;
		akkuStr = 0;
		for(uint32_t l = 0;l < 4 && l < str->length;l++){
			akkuStr = akkuStr * 256 + str->string[l];
		}
		if(akkuStr == 0x6966) // if
			tree->tokenType = INTE_IF;
		else if(akkuStr == 0x656C7365) // else
			tree->tokenType = INTE_ELSE;
		else if(akkuStr == 0x656C6966) // elif
			tree->tokenType = INTE_ELSE_IF;
		else if(akkuStr == 0x646566) // def
			tree->tokenType = INTE_DEF;
		else if(akkuStr == 0x666f72) // for
			tree->tokenType = INTE_FOR;
		else if(akkuStr == 0x6c6f6f70) // loop
			tree->tokenType = INTE_LOOP;
		else if(akkuStr == 0x6e696c) // nil
			tree->tokenType = INTE_LOOP;
		else if(akkuStr == 0x726574) // ret
			tree->tokenType = INTE_LOOP;
		if(tree->tokenType != INTE_KEY_WORD){
			tree->data = NULL;
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

void interpStatementIntoTree(
		InterpTree *base,InterpTree *ltree,InterpTree *rtree,bool pretendFull){
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
			if(pretendFull)
				base->flags |= 966;
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
			if(pretendFull)
				base->flags |= 96;
		}
	}
}
void interpParseLRStatement(
		UtilSharedStruct2 tokenList,
		int32_t lower,int32_t upper,
		InterpToken tLower,InterpToken tUpper,
		bool pretendFull
		){
	// TODO adding funny features!
	int32_t idx;
	InterpTree *tree;
	int32_t tempIdx1,tempIdx2;
	tempIdx1 = 0;
	tempIdx2 = 0;
	InterpTree *useLTree,*useRTree;

	for(idx = lower;idx < upper;idx++){
		useLTree = NULL;
		useRTree = NULL;
		idx = interpGetPosibleToken(tokenList,lower,upper,idx,1);
		// ???
		tree = &((InterpTree*)tokenList.vptr)[idx];
		if(tree->tokenType > tLower && tree->tokenType < tUpper){
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
			interpStatementIntoTree(tree,useLTree,useRTree,pretendFull);
			// TODO
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
		// TODO turn into seperate functions
		if(	tree->tokenType == INTE_BRACK_OPEN ||
			tree->tokenType == INTE_SQ_BRACK_OPEN
				){
			if(bracketDepth == 0){
				tempIdx1 = idx;
			}
			bracketDepth++;
		}
		// tree->tokenType == INTE_BRACK_CLOSE ||
		if(	tree->tokenType == INTE_BRACK_CLOSE ||
			tree->tokenType == INTE_SQ_BRACK_CLOSE
				){
			bracketDepth--;
			if(bracketDepth > 0)
				continue;
			tempIdx2 = idx;
			interpParseStatement(tokenList,tempIdx1,tempIdx2);
			// remove ( and ) from beeing used
			//useLTree = &((InterpTree*)tokenList.vptr)[tempIdx1];
			useRTree = &((InterpTree*)tokenList.vptr)[tempIdx1];
			// test later for indexing
			if(useLTree->tokenType != INTE_SQ_BRACK_CLOSE){
				useRTree->flags |= 128;
			}
			tree->flags |= 128;
			if(tempIdx1 - 1 < 0){
				useRTree->flags |= 128;
				continue;
			}
			// test if it sould be an function
			useLTree = &((InterpTree*)tokenList.vptr)[tempIdx1 - 1];
			if(useLTree->tokenType != INTE_KEY_WORD){
				useRTree->flags |= 128;
				continue;
			}
			// is this used?
			if((useLTree->flags & 128) > 0){
				useRTree->flags |= 128;
				continue;
			}
			// used for indexing
			if(tree->tokenType == INTE_SQ_BRACK_CLOSE){
				useRTree->ltree = useLTree;
				int32_t position = interpGetPosibleToken(
						tokenList,lower,upper,tempIdx1,1);
				useRTree->rtree = 
					&((InterpTree*)tokenList.vptr)[position];
				continue;
			}
			// used for calling!
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
	// parse word.subword
	interpParseLRStatement(tokenList,lower,upper,INTEV_PRIO0,INTEV_PRIO1,false);
	// parse * / %
	interpParseLRStatement(tokenList,lower,upper,INTEV_PRIO1,INTEV_PRIO2,true);
	// parse + -
	interpParseLRStatement(tokenList,lower,upper,INTEV_PRIO2,INTEV_PRIO3,false);
	return idx;
}

int32_t interpParseOperation_RET(
		UtilSharedStruct2 tokenList,
		int32_t current,int32_t lower,int32_t upper){
	InterpTree *tree;
	// the ret keyword
	tree = &((InterpTree*)tokenList.vptr)[current];
	int32_t tempIdx;
	InterpTree *rtree;
	// the stuff to return?
	tempIdx = interpGetPosibleToken(tokenList,lower,upper,current + 1,1);
	rtree = &((InterpTree*)tokenList.vptr)[tempIdx];
	if(rtree->tokenType < INTE_SEMI){
		tree->rtree = rtree;
		rtree->flags |= 128;
	}
	else{
		// TODO adding error logging
	}
	// goto next statement (hopfully!)
	current = interpGetPosibleToken(tokenList,lower,upper,tempIdx,1);
	rtree = &((InterpTree*)tokenList.vptr)[current];
	// maybe after an semicolon
	if(rtree->tokenType == INTE_SEMI)current++;
	return current;
}
int32_t interpParseOperation_WORD(
		UtilSharedStruct2 tokenList,
		int32_t current,int32_t lower,int32_t upper){
	InterpTree *tree, *ltree, *rtree;
	tree = &((InterpTree*)tokenList.vptr)[current];
	// the ret keyword
	int32_t tempIdx0,tempIdx1,tempIdx2,tempIdx3;
	tempIdx0 = current;
	int32_t depth = 0;
	int32_t idx;
	// ltree 4 condition
	for(idx = current + 1;idx < upper;idx++){
		idx = interpGetPosibleToken(tokenList,lower,upper,idx,1);
		ltree = &((InterpTree*)tokenList.vptr)[idx];
		if(ltree->tokenType == INTE_BRACK_OPEN){
			if(depth == 0){
				tempIdx0 = idx;
				ltree->flags |= 128;
			}
			depth++;
		}
		if(ltree->tokenType == INTE_BRACK_CLOSE){
			depth--;
			if(depth != 0)
				continue;
			ltree->flags |= 128;
			interpParseStatement(tokenList,tempIdx0 + 1,idx - 1);
			break;
		}
	}
	tempIdx1 = interpGetPosibleToken(tokenList,lower,upper,current + 1,1);
	ltree = &((InterpTree*)tokenList.vptr)[tempIdx1];
	ltree->flags |= 128;
	tree->data = ltree;
	tempIdx2 = interpGetPosibleToken(tokenList,lower,upper,idx + 1,1);
	// rtree 4 when the condition holds
	for(idx = tempIdx2;idx < upper;idx++){
		idx = interpGetPosibleToken(tokenList,lower,upper,idx,1);
		ltree = &((InterpTree*)tokenList.vptr)[idx];
		if(ltree->tokenType == INTE_BRACK_OPEN){
			if(depth == 0)
				tempIdx2 = idx;
			depth++;
		}
		if(ltree->tokenType == INTE_BRACK_CLOSE){
			depth--;
			if(depth != 0)
				continue;
			// redo this tree
			interpParseOperation(tokenList,tempIdx2 + 1,idx - 1);
			break;
		}
	}
	tempIdx3 = interpGetPosibleToken(tokenList,lower,upper,tempIdx2 + 1,1);
	ltree = &((InterpTree*)tokenList.vptr)[tempIdx3];
	tree->ltree = ltree;

	tempIdx3 = interpGetPosibleToken(tokenList,lower,upper,idx + 1,1);
	rtree = &((InterpTree*)tokenList.vptr)[tempIdx3];
	tree->rtree = rtree;
	return tempIdx3;
}
int32_t interpParseOperation_ELSE(
		UtilSharedStruct2 tokenList,
		int32_t current,int32_t lower,int32_t upper){
	InterpTree *tree, *ltree, *rtree;
	tree = &((InterpTree*)tokenList.vptr)[current];
	// the ret keyword
	int32_t tempIdx0,tempIdx1;
	tempIdx0 = current;
	int32_t depth = 0;
	int32_t idx;
	// rtree 4 when the condition holds
	for(idx = current + 1;idx < upper;idx++){
		idx = interpGetPosibleToken(tokenList,lower,upper,idx,1);
		ltree = &((InterpTree*)tokenList.vptr)[idx];
		if(ltree->tokenType == INTE_BRACK_OPEN){
			if(depth == 0)
				tempIdx0 = idx;
			depth++;
		}
		if(ltree->tokenType == INTE_BRACK_CLOSE){
			depth--;
			if(depth != 0)
				continue;
			// redo this tree
			interpParseOperation(tokenList,tempIdx0 + 1,idx - 1);
			break;
		}
	}
	tempIdx0 = interpGetPosibleToken(tokenList,lower,upper,tempIdx0 + 1,1);
	ltree = &((InterpTree*)tokenList.vptr)[tempIdx0];
	tree->ltree = ltree;

	tempIdx1 = interpGetPosibleToken(tokenList,lower,upper,idx + 1,1);
	rtree = &((InterpTree*)tokenList.vptr)[tempIdx1];
	tree->rtree = rtree;
	return tempIdx1;
}
int32_t interpParseOperation_FOR(
		UtilSharedStruct2 tokenList,
		int32_t current,int32_t lower,int32_t upper){
	InterpTree *tree, *ltree, *rtree;
	tree = &((InterpTree*)tokenList.vptr)[current];
	// the ret keyword
	int32_t tempIdx0,tempIdx1,tempIdx2,tempIdx3;
	tempIdx0 = current;
	int32_t depth = 0;
	int32_t idx;
	// ltree 4 condition
	for(idx = current + 1;idx < upper;idx++){
		// TODO add the l op of = to this
		idx = interpGetPosibleToken(tokenList,lower,upper,idx,1);
		ltree = &((InterpTree*)tokenList.vptr)[idx];
		if(ltree->tokenType == INTE_BRACK_OPEN){
			if(depth == 0){
				tempIdx0 = idx;
				ltree->flags |= 128;
			}
			depth++;
		}
		if(ltree->tokenType == INTE_BRACK_CLOSE){
			depth--;
			if(depth != 0)
				continue;
			ltree->flags |= 128;
			interpParseStatement(tokenList,tempIdx0 + 1,idx - 1);
			break;
		}
	}
	tempIdx1 = interpGetPosibleToken(tokenList,lower,upper,current + 1,1);
	ltree = &((InterpTree*)tokenList.vptr)[tempIdx1];
	ltree->flags |= 128;
	tree->data = ltree;
	tempIdx2 = interpGetPosibleToken(tokenList,lower,upper,idx + 1,1);
	// rtree 4 when the condition holds
	for(idx = tempIdx2;idx < upper;idx++){
		idx = interpGetPosibleToken(tokenList,lower,upper,idx,1);
		ltree = &((InterpTree*)tokenList.vptr)[idx];
		if(ltree->tokenType == INTE_BRACK_OPEN){
			if(depth == 0)
				tempIdx2 = idx;
			depth++;
		}
		if(ltree->tokenType == INTE_BRACK_CLOSE){
			depth--;
			if(depth != 0)
				continue;
			// redo this tree
			interpParseOperation(tokenList,tempIdx2 + 1,idx - 1);
			// TODO could add the = check!
			// will be an FOR_EACH like function
			break;
		}
	}
	tempIdx3 = interpGetPosibleToken(tokenList,lower,upper,tempIdx2 + 1,1);
	ltree = &((InterpTree*)tokenList.vptr)[tempIdx3];
	tree->ltree = ltree;

	tempIdx3 = interpGetPosibleToken(tokenList,lower,upper,idx + 1,1);
	rtree = &((InterpTree*)tokenList.vptr)[tempIdx3];
	tree->rtree = rtree;
	return tempIdx3;
}
int32_t interpParseOperation_ASSIGN(
		UtilSharedStruct2 tokenList,
		int32_t current,int32_t lower,int32_t upper){
	InterpTree *tree, *ltree, *rtree;
	tree = &((InterpTree*)tokenList.vptr)[current];
	// the ret keyword
	int32_t tempIdx0,tempIdx1;
	tempIdx0 = current;
	int32_t idx;
	// rtree 4 when the condition holds
	ltree = NULL;
	for(idx = current + 1;idx < upper;idx++){
		idx = interpGetPosibleToken(tokenList,lower,upper,idx,1);
		tree = &((InterpTree*)tokenList.vptr)[idx];
		if(tree->tokenType == INTE_SEMI){
			idx--;
			break;
		}
		if(tree->tokenType == INTE_EQUALS){
			rtree = &((InterpTree*)tokenList.vptr)[tempIdx0];
			tree->ltree = rtree;
			rtree->flags |= 128;
			tree->data = ltree;
			//
			if(ltree == NULL){
				ltree->flags |= 128;
			}
			ltree = tree;
			tempIdx1 = idx;
		}
		tempIdx0 = idx;
	}
	interpParseStatement(tokenList,tempIdx1 + 1,idx);
	tempIdx1 = interpGetPosibleToken(tokenList,lower,upper,idx + 1,1);
	return idx + 2; // after semicolon
}
int32_t interpParseOperation_DEF(
		UtilSharedStruct2 tokenList,
		int32_t current,int32_t lower,int32_t upper){
	// use ref: 375 or
	// tree->tokenType == INTE_BRACK_CLOSE ||
	InterpTree *tree, *ltree, *rtree;
	tree = &((InterpTree*)tokenList.vptr)[current];
	// the ret keyword
	int32_t tempIdx0,tempIdx1,tempIdx2,tempIdx3;
	tempIdx0 = current;
	int32_t depth = 0;
	int32_t idx;
	int32_t argCount = 0;
	// counting arguments?
	for(idx = current + 1;idx < upper;idx++){
		idx = interpGetPosibleToken(tokenList,lower,upper,idx,1);
		ltree = &((InterpTree*)tokenList.vptr)[idx];
		if(ltree->tokenType == INTE_BRACK_OPEN){
			if(depth == 0){
				tempIdx0 = idx;
				ltree->flags |= 128;
			}
			// TODO say error!
			if(depth > 1){}
			depth++;
		}
		if(depth == 1 && ltree->tokenType == INTE_KEY_WORD)
			argCount++;
		if(ltree->tokenType == INTE_BRACK_CLOSE){
			depth--;
			if(depth != 0)
				continue;
			//ltree->flags |= 128;
			//interpParseStatement(tokenList,tempIdx0 + 1,idx - 1);
			break;
		}
	}
	tree->argumentLength = argCount;
	void **lengthing = malloc(sizeof(void*) * argCount);
	argCount = 0;
	// adding params into array
	for(idx = current + 1;idx < upper;idx++){
		idx = interpGetPosibleToken(tokenList,lower,upper,idx,1);
		ltree = &((InterpTree*)tokenList.vptr)[idx];
		if(ltree->tokenType == INTE_BRACK_OPEN){
			if(depth == 0){
				tempIdx0 = idx;
				ltree->flags |= 128;
			}
			// TODO say error!
			if(depth > 1){}
			depth++;
		}
		if(depth == 1 && ltree->tokenType == INTE_KEY_WORD){
			lengthing[argCount] = ltree;
			argCount++;
		}
		if(ltree->tokenType == INTE_BRACK_CLOSE){
			depth--;
			if(depth != 0)
				continue;
			//ltree->flags |= 128;
			//interpParseStatement(tokenList,tempIdx0 + 1,idx - 1);
			break;
		}
	}
	// after 
	tree->data = lengthing;
	//
	tempIdx2 = interpGetPosibleToken(tokenList,lower,upper,idx + 1,1);
	// rtree 4 when the condition holds
	for(idx = tempIdx2;idx < upper;idx++){
		idx = interpGetPosibleToken(tokenList,lower,upper,idx,1);
		ltree = &((InterpTree*)tokenList.vptr)[idx];
		if(ltree->tokenType == INTE_BRACK_OPEN){
			if(depth == 0)
				tempIdx2 = idx;
			depth++;
		}
		if(ltree->tokenType == INTE_BRACK_CLOSE){
			depth--;
			if(depth != 0)
				continue;
			// redo this tree
			interpParseOperation(tokenList,tempIdx2 + 1,idx - 1);
			break;
		}
	}
	tempIdx3 = interpGetPosibleToken(tokenList,lower,upper,tempIdx2 + 1,1);
	ltree = &((InterpTree*)tokenList.vptr)[tempIdx3];
	tree->ltree = ltree;

	tempIdx3 = interpGetPosibleToken(tokenList,lower,upper,idx + 1,1);
	rtree = &((InterpTree*)tokenList.vptr)[tempIdx3];
	tree->rtree = rtree;
	return tempIdx3;
}
// note:
//  rstatement: just statement
//  lstatement: todo
//  functions: see lower
//  keywords:
//    return will use rstateent
//    if like functions, if(rstatement)(operations)
//    for is funny, but also for(special)(operations)
//
// TODO seperate this stuff?
// 1. -assignment/capculation-ish: interpParseOperation_ASSIGN
// 2. -return ?			: interpParseOperation_RET
// 3. -if/loop/..?		: interpParseOperation_WORD
// 4. -else			: interpParseOperation_ELSE
// 5. -for (TODO)		: interpParseOperation_FOR
// 6. -def			: interpParseOperation_DEF
int32_t interpParseOperation(UtilSharedStruct2 tokenList,int32_t lower,int32_t upper){
	int32_t idx,lastIdx;
	int32_t tempIdx0,tempIdx1,tempIdx2,tempIdx3;
	int32_t tempIdxNext;
	lastIdx = lower;
	tempIdx0 = lower;
	tempIdxNext = lower;
	tempIdx2 = lower;
	InterpTree *tree;
	InterpTree *useLTree,*useRTree;
	useRTree = NULL;
	// looking for = and ;
	for(idx = lower;idx < upper;idx++,lastIdx = idx){
		idx = interpGetPosibleToken(tokenList,lower,upper,idx,1);
		tree = &((InterpTree*)tokenList.vptr)[idx];
		// this could be an switch?
		if(tree->tokenType == INTE_RET){
			idx = interpParseOperation_RET(tokenList,idx,lower,upper);
			tempIdxNext = idx;
			idx--;
			continue;
		}
		if(tree->tokenType >= INTE_IF && tree->tokenType <= INTE_ELSE_IF){
			idx = interpParseOperation_WORD(tokenList,idx,lower,upper);
			tempIdxNext = idx;
			idx--;
			continue;
		}
		if(tree->tokenType == INTE_ELSE){
			idx = interpParseOperation_WORD(tokenList,idx,lower,upper);
			tempIdxNext = idx;
			idx--;
			continue;
		}
		if(tree->tokenType == INTE_FOR){
			// TODO
			idx = interpParseOperation_FOR(tokenList,idx,lower,upper);
			tempIdxNext = idx;
			idx--;
			continue;
		}
		if(tree->tokenType == INTE_SEMI){
			idx = interpParseOperation_ASSIGN(tokenList,tempIdxNext,lower,upper);
			tempIdxNext = idx;
			idx--;
			continue;
		}
		if(tree->tokenType == INTE_DEF){
			idx = interpParseOperation_DEF(tokenList,tempIdxNext,lower,upper);
			tempIdxNext = idx;
			idx--;
			continue;
		}
		/*
		if(tree->tokenType == INTE_EQUALS){
			useLTree = &((InterpTree*)tokenList.vptr)[lastIdx];
			useLTree->flags |= 128;
			tree->ltree = useLTree;
			// allowing a = b = ...;
			if(useRTree != NULL){
				tree->data = useRTree;
				useRTree->flags |= 128;
			}
			tempIdx2 = idx;
			useRTree = tree;
		}
		if(tree->tokenType == INTE_RET){
			tempIdx2 = idx;
			useRTree = tree;
		}
		if(tree->tokenType != INTE_SEMI){
			// TODO use tempIdxNext
			continue;
		}
		// 
		interpParseStatement(tokenList,tempIdx2 + 1,idx);
		// get created tree
		tempIdx1 = interpGetPosibleToken(tokenList,tempIdx0,idx,tempIdx2,1);
		useRTree = &((InterpTree*)tokenList.vptr)[tempIdx1];
		// put this under ; and consume!
		if(tempIdx1 == tempIdx2){
			useLTree = &((InterpTree*)tokenList.vptr)[tempIdx2];
			useLTree->rtree = useRTree;
			tree->ltree = useLTree;
			useLTree->flags |= 128;
		}
		else{
			tree->ltree = useRTree;
			useRTree->flags |= 128;
		}
		// move to next instruction
		tempIdx0 = idx + 1;
		tempIdx1 = tempIdx0 + 1;
		tempIdx2 = tempIdx0 + 1;
		useLTree = NULL;
		useRTree = NULL;
		// */
	}
	return idx;
}
// could also get a scope?
int32_t interpParseFunctions(UtilSharedStruct2 tokenList,ScopeObject *globalData){
	int32_t idx;
	int32_t tempIdx0,tempIdx1,tempIdx2;
	InterpTree *tree;
	tempIdx0 = 0;
	tempIdx1 = 0;
	tempIdx2 = 0;
	int32_t bracketDepth = 0;
	int32_t state = 0;
	// function are:
	// def name(param param)(
	//   op1;
	//   op2;
	// )
	// could also:
	// name2 = stuff;
	// name3 = stuff;
	for(idx = 0;idx < tokenList.vint;idx++){
		tree = &((InterpTree*)tokenList.vptr)[idx];
		if(tree->tokenType == INTE_DEF){
			tempIdx0 = idx;
			state = 4;
		}
		if(tree->tokenType == INTE_KEY_WORD){
			tempIdx0 = idx;
			state = 1;
		}
		if(tree->tokenType == INTE_BRACK_OPEN){
			if(bracketDepth == 0)
				tempIdx1 = idx;
			bracketDepth++;
		}
		if(tree->tokenType == INTE_SEMI && state == 1){
			// adding consts
			state = 0;
		}
		if(tree->tokenType == INTE_BRACK_CLOSE){
			bracketDepth--;
		}
		else if(bracketDepth >= 0){
			if(state == 4){
				tempIdx2 = idx;
				state = 2;
			}
			continue;
		}
		if(state == 2){
			// adding functions
			state = 0;
		}
	}
	return 0;
}

