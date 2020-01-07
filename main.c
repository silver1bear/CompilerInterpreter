#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Lexer.h"
#include "NameList.h"
#include "Parser.h"

int initLex(char* fname){
	char vName[128+1];

	Z=0;

	strcpy(vName, fname);
	if(strstr(vName, ".pl0")==NULL) strcat(vName, ".pl0");

	pIF = fopen(vName, "r+t");
	if(pIF!=NULL) {return OK;}
	return FAIL;
}

tMorph* Lex(void){
	int cClass;
	Z = state = 0;
	Morph = MorphInit;
	Morph.PosLine = line;
	Morph.PosCol = col;
	Morph.MLen = 0;
	pBuf = vBuf;

	//Check for EOF (temporary)
	if(c == EOF){
		Morph.Val.Symb = mcEOF;
		return &Morph;
	}

	do{
		if(wroteLastC)
			readc();
		cClass = cLookupTable[c];   //Determine character class

		//Handle whitespace
		if(cClass == 0){
			break;
		}

		state = edgeLookupTable[state][cClass]; //Determine state we're currently in

		if(state == 0 && wroteLastC != 0) {
			wroteLastC = 0;
			break;
		} else{
			writec();
			wroteLastC = 1;
		}

		finalize();

		//printf("Zeichen: %c\n", c);
	}while(state != 0);

	int keywordIdx = 0;
	if(Morph.MC == mcIdent)
		checkKeyword(&Morph);

	if(Morph.MC == mcSymb && Morph.MLen == 1){
		Morph.Val.Symb = *(Morph.Val.pStr);
	}

	if(Morph.Val.Symb == mcEmpty)
		return Lex();
	else return &Morph;
}

static void finalize(void){
	switch(state){
		case 0: //Read special character
			Morph.MC        = mcSymb;
			Morph.Val.pStr  = vBuf;
			break;
		case 1: //Read letter
			Morph.MC        = mcIdent;
			Morph.Val.pStr  = vBuf;
			break;
		case 2: //Read numeral
			Morph.MC        = mcNum;
			Morph.Val.Num   = atol(vBuf);
			break;
		case 3: //Read ':'
			Morph.MC        = mcSymb;
			Morph.Val.pStr  = vBuf;
			break;
		case 4: //Read '<'
			Morph.MC        = mcSymb;
			Morph.Val.pStr  = vBuf;
			break;
		case 5: //Read '>'
			Morph.MC        = mcSymb;
			Morph.Val.pStr  = vBuf;
			break;
		case 6: //Read ":="
			Morph.MC        = mcSymb;
			Morph.Val.Symb  = zERG;
			break;
		case 7: //Read "<="
			Morph.MC        = mcSymb;
			Morph.Val.Symb  = zLE;
			break;
		case 8: //Read ">="
			Morph.MC        = mcSymb;
			Morph.Val.Symb  = zGE;
			break;
	}

	Morph.MLen++;
	return;
}

void readc  (void){
	c=fgetc(pIF);

	if (c == '\n') line++, col=0;
	else col++;

	return;
}

void writec(void){
	//Write to buffer
	if(isalpha(c))
		*pBuf = (char)toupper(c);
	else
		*pBuf = (char)c;

	*(++pBuf) = 0; //Move pointer forward

	return;
}
/*
static void fl(void){
	return;
}

//---- schreiben als Grossbuchstabe, lesen ----
static void fgl (void){
	*pBuf=(char)toupper(c);// oder  *Buf=(char)c&0xdf;
	*(++pBuf)=0;
	Morph.MLen++;
	fl();
}

//---- schreiben, lesen ----
static void fsl (void){
	*pBuf=(char)c;
	*(++pBuf)=0;
	fl();
}

//---- schreiben, lesen, beenden ----
static void fslb(void){
	fsl();fb();
}

static void fb  (){
	int i,j;
	switch (lastZ){
		//Identifier
		case 1:
			Morph.MC = mcSymb;
			if(Morph.Val.pStr == NULL) Morph.Val.pStr = vBuf;
			break;
			// Doppelpunkt
		case 3:
			Morph.Val.Symb = vBuf[0];
			Morph.MC = mcSymb;
			break;
			//Kleiner
		case 4:
			Morph.Val.Symb = vBuf[0];
			Morph.MC = mcSymb;
			break;
			//Groesser
		case 5:
			Morph.Val.Symb = vBuf[0];
			Morph.MC = mcSymb;
			break;
		case 0:
			Morph.Val.Symb=vBuf[0];
			Morph.MC =mcSymb;
			break;
			//Zahl
		case 2: Morph.Val.Num=atol(vBuf);
			Morph.MC =mcNum;
			break;
			//Ergibtzeichen
		case 6: Morph.Val.Symb=(long)zERG;
			Morph.MC =mcSymb;
			break;
			//KleinerGleich
		case 7: Morph.Val.Symb=(long)zLE;
			Morph.MC =mcSymb;
			break;
			//GroesserGleich
		case 8: Morph.Val.Symb=(long)zGE;
			Morph.MC =mcSymb;
			break;
	}
	Z = 0;
}
*/
int checkKeyword(tMorph* m){
	int c, idx;
	for(c = 0; c < MAXKEYWORDCLASSLENGTH; c++){
		if((idx = keywordClasses[m->MLen][c]) >= 0 && !strcmp(m->Val.pStr, keyWords[idx])){
			m->MC = mcSymb;
			m->Val.Symb = (tZS)idx+131;
			return idx;
		}
	}
	return NOKEYWORD;
}

int depth = 0;

void LookupGraph(tBog* pBog){
	int i, j;

	for(i = 0; i < 13; i+=2){
		if(pBog >= endStates[i] && pBog <= endStates[i+1]){
            printf("==================================================\n");
			printf("Depth: %d Graph: %s.\n", depth, graphNames[i]);
            printf("==================================================\n\n");
			return;
		}
	}
}

void PrintMorph(tMorph* m){
    int keywordIdx = 0;
    
    switch(m->MC){
		case mcSymb:
			printf("Morphem %-10s: %s\n", "mcSymb", vBuf);
			break;
		case mcNum:
			printf("Morphem %-10s: %ld\n", "mcNum", m->Val.Num);
			break;
		case mcIdent:
			printf("Morphem %-10s: %s\n", "mcIdent", m->Val.pStr);
			if((keywordIdx = checkKeyword(m)) != -1)
				printf("Keyword %-10s\n", keyWords[keywordIdx]);
			break;
		case mcEOF:
        case mcEmpty:
            break;
    }
}

int parse(tBog* pGraph)
{
	depth++;
	tBog* pBog=pGraph;
	int succ=0;

	//DEBUG
	//LookupGraph(pBog);

	if (Morph.MC==mcEmpty)  Lex();
	while(1)
	{
		switch(pBog->BgD) {
			case BgNl:succ=1;                            break;
			case BgSy:succ=(Morph.Val.Symb==pBog->BgX.S);break;
			case BgMo:succ=(Morph.MC==(tMC)pBog->BgX.M); break;
			case BgGr:succ=parse(pBog->BgX.G);           break;
			case BgEn:depth--; return 1;  /* Ende erreichet - Erfolg */
		}

		//PrintMorph(&Morph);
		
		if (succ && pBog->fx!=NULL) succ=pBog->fx();
		if (!succ)/* Alternativbogen probieren */
			if (pBog->iAlt != 0)
				pBog=pGraph+pBog->iAlt;
			else return FAIL;
		else /* Morphem formal akzeptiert (eaten) */
		{
			//printf("Morphem formal akzeptiert.\n");
			if (pBog->BgD & BgSy || pBog->BgD & BgMo) Lex();
			pBog=pGraph+pBog->iNext;
		}
	}/* while */
	depth--;
}

tVar* CreateVar(void){
    tVar* newVar 		= malloc(sizeof(tVar));
    
    newVar->Kz 			= KzVar;
    newVar->Dspl 		= procList->SpzzVar;
    
	procList->SpzzVar	+=4;
    
    return newVar;
}

tConst* createConst(long Val){
    tConst* newConst = (tConst*)malloc(sizeof(tConst));
    
    newConst->Kz = KzConst;
    newConst->Val = Val;
    newConst->Idx = ConstCounter++;
    
    //Not sure if correct
    void* ret = realloc(ConstBlock, sizeof(int) * ConstCounter);
    if(ret == NULL) printf("Realloc in const-Creation failed\n");
    ConstBlock[ConstCounter-1] = Val;
	
	return newConst;
}

tBez* createBez(char* pBez){
    
    tBez* newBez = malloc(sizeof(tBez));
    
	newBez->nxt 	= procList->pLBez;
    newBez->IdxProc = procCounter; //TODO: Check if procCounter or procCounter+1
    newBez->Len 	= strlen(pBez);
    newBez->pName 	= (char*)malloc(newBez->Len + 1);
    strcpy(newBez->pName, pBez);
    newBez->pObj	= NULL;
    
    procList->pLBez = newBez;
    
    return newBez;
}

tProc* createProc(tProc* pParent){
	/*
    if(procCounter == 0){
        procList = (void*)malloc(sizeof(tProc));
    } else{
        
        // Neue Prozedurbeschreibung anlegen
        // Prozedurbeschreibung in Bezeichner verlinken
        
        tBez* oldListStart = procList->pLBez;
        procList->pLBez = malloc(sizeof(tProc));
    }
    */
    
    tProc* newProc = (tProc*)malloc(sizeof(tProc));
    
    ((tProc*)(newProc))->Kz 		= KzProc;
    ((tProc*)(newProc))->IdxProc 	= procCounter++;
    ((tProc*)(newProc))->pParent 	= pParent;
    ((tProc*)(newProc))->pLBez 		= NULL;
    ((tProc*)(newProc))->SpzzVar 	= 0;
    
    return newProc;
}

int NewVar(){
	printf("New Variable\n");
	
	//TODO: search Variable
	
	tBez* newBezeichner = createBez(Morph.Val.pStr);
	newBezeichner->Kz = KzVar;
	
	printf("Created Bezeichner\n");
	
	procList->pLBez->pObj = CreateVar();
	
	printf("Created new Variable %s with relative address %d\n\n\n", procList->pLBez->pName, ((tVar*)(procList->pLBez->pObj))->Dspl);
	
	return 1;
}

int NewConstBez(){
	printf("New Constant\n");
	
	//TODO: search Const
	
	tBez* newBezeichner = createBez(Morph.Val.pStr);
	newBezeichner->Kz = KzConst;
	
	printf("Created Bezeichner\n");
	return 1;
}

int NewConst(){
	procList->pLBez->pObj = createConst(Morph.Val.Num);
	
	printf("Created new Constant %s with value: %ld\n\n\n", procList->pLBez->pName, ((tConst*)(procList->pLBez->pObj))->Val);
	
	return 1;
}

int newProc(){
    // Suche nach Bezeichner
    
    printf("New Procedure\n");
    
    // if not found
    tBez* newBezeichner = createBez(Morph.Val.pStr);
    newBezeichner->Kz = KzProc;
    
    printf("Created Bezeichner\n");
    
    tProc* newProcedure = createProc(procList);

	newBezeichner->pObj = newProcedure;
	procList = newProcedure;
    
    printf("Created Procedure-Block with no. %d and name %s\n\n\n", procCounter, newBezeichner->pName);
    
    
    //On error: return false
    return procCounter;
}

void newProg(){

	printf("New Program\n");
	
	tProc* newProcedure = createProc(NULL);
	procList = newProcedure;
	
	char* name = "Program";
	tBez* newBezeichner = createBez(name);
	
	printf("Created Bezeichner\n");
	
	printf("Created Procedure-Block with no. %d and name %s\n\n\n", procCounter, newBezeichner->pName);
}


int main(int argc, char* argv[]){

	if(argc != 2){
		printf("No source file passed");
	}

	tMorph* tmp;
	int debugCounter = 0, keywordIdx = 0;
	ConstBlock = (int*)malloc(sizeof(int));
	ConstCounter = 0;

	printf("%s\n", (initLex(argv[1]))?"InitLex failed.\n":"");

	newProg();
	parse(gProgram);
    
    printf("Done.\n");

/*
	do{
		tmp = Lex();

		switch(tmp->MC){
			case mcSymb:
				printf("Morphem %-10s: %s\n", "mcSymb", vBuf);
				break;
			case mcNum:
				printf("Morphem %-10s: %ld\n", "mcNum", tmp->Val.Num);
				break;
			case mcIdent:
				printf("Morphem %-10s: %s\n", "mcIdent", tmp->Val.pStr);
				if((keywordIdx = checkKeyword(tmp)) != -1)
					printf("Keyword %-10s\n", keyWords[keywordIdx]);
				break;
			case mcEOF:
			case mcEmpty:
				break;
		}

		debugCounter++;
	} while(debugCounter <= 200);
*/

	return 0;
}
