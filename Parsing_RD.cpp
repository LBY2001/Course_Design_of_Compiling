#include<iostream>
#include<fstream>
#include<string>
#include<unordered_map>
#include<algorithm>
#include<set>
#include<vector>
#include<iomanip>
#include<sstream>
#include<stdio.h>
#include<stdlib.h>
#include"Parsing_RD.h"

FILE* fin;
FILE* flisting;
int tokenNum;		//读到第几个token序列了
bool Error;
Token token;		//正在判断的token序列
int line0;			//在代码中的行数
string temp_name;	//临时存变量名
ofstream textfile;	//输出文件
FILE* fp;			//读入文件

void RecursiveDescentParsing::initial()
{
	Error = false;
	tokenNum = 0;
	line0 = 0;
	//fopen_s(&flisting, "parseError.txt", "w");
	textfile.open("parseError.txt");
	fopen_s(&fp, "tokenList.txt", "rb");
}

void RecursiveDescentParsing::ReadNextToken()
{
	//FILE* fp;
	//fopen_s(&fp, "tokenList.txt", "rb");
	if (fp == NULL)
	{
		printf("tokenList打开失败!\n");
		Error = true;
		exit(0);
	}
	//fseek(fp, tokenNum * (sizeof(Token) + 2 * sizeof(' ') + sizeof('\r')), 0);
	char tempChar[100];
	fscanf_s(fp,"%d%d%s",&token.lineShow, &token.word.Lex, tempChar, 100);
	string a(tempChar);
	token.word.Sem = a;
	char temp;
	fscanf_s(fp, "%c", &temp);
	tokenNum++;

}

void RecursiveDescentParsing::syntaxError(string errorMessage)
{
	textfile << "Syntax error, line " << token.lineShow << " : " << errorMessage << endl;
	//fprintf(flisting, "Syntax error, line %d : %s\n\n", token.lineShow, errorMessage);
	Error = true;
}

void RecursiveDescentParsing::match(LexType lt)
{
	if (token.word.Lex == lt)
	{
		ReadNextToken();
		line0 = token.lineShow;
	}
	else
	{
		syntaxError("符号错误，不匹配");
		ReadNextToken();
	}
}

void RecursiveDescentParsing::fileClose()
{
	fclose(fp);
	textfile.close();
}

TreeNode* RecursiveDescentParsing::parse(void)
{
	ReadNextToken();

	TreeNode* t = NULL;
	t = Program();

	//if (token.word.Lex != ENDFILE1)
	//	syntaxError("Code ends before file", token.lineShow);
	return t;
}

TreeNode* RecursiveDescentParsing::Program(void)
{
	TreeNode* t = ProgramHead();
	TreeNode* q = DeclarePart();
	TreeNode* s = ProgramBody();

	//建立根节点
	TreeNode* root = new TreeNode;
	if (root == NULL)
	{
		fprintf(flisting, "内存溢出：第 0 行\n");
		Error = true;
		exit(0);
	}
	else {
		for (int i = 0; i < 3; i++)
			root->child[i] = NULL;
		root->sibling = NULL;
		root->nodekind = ProK;
		root->lineno = 0;
		for (int i = 0; i < 10; i++)
		{
			root->name[i] = "";
			root->table[i] = NULL;
		}
	}


	if (root != NULL)
	{
		root->lineno = 0;
		if (t != NULL) 
			root->child[0] = t;
		else 
			syntaxError("缺少程序头");
		if (q != NULL) root->child[1] = q;
		if (s != NULL) root->child[2] = s;
		else syntaxError("缺少程序头");
	}
	match(DOT);

	return root;
}

TreeNode* RecursiveDescentParsing::ProgramHead(void)
{
	TreeNode* t = new TreeNode;
	if (t == NULL)
	{
		syntaxError("内存溢出");
		Error = true;
	}
	else {
		for (int i = 0; i < 3; i++) 
			t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = PheadK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "";
			t->table[i] = NULL;
		}
	}
	match(PROGRAM);
	if ((t != NULL) && (token.word.Lex == ID))
	{
		t->lineno = 0;
		t->name[0] = token.word.Sem;
	}
	match(ID);
	return t;
}

TreeNode* RecursiveDescentParsing::DeclarePart(void)
{
	//newDecANode
	TreeNode* typeP = new TreeNode;
	if (typeP == NULL)
	{
		syntaxError("内存溢出");
		Error = true;
	}
	else {
		for (int i = 0; i < 3; i++) 
			typeP->child[i] = NULL;
		typeP->sibling = NULL;
		typeP->nodekind = TypeK;
		typeP->lineno = token.lineShow;
		for (int i = 0; i < 10; i++)
		{
			typeP->name[i] = "\0";
			typeP->table[i] = NULL;
		}
	}

	TreeNode* pp = typeP;
	if (typeP != NULL)
	{
		typeP->lineno = 0;
		TreeNode* tp1 = TypeDec();
		if (tp1 != NULL)
			typeP->child[0] = tp1;
		else
		{
			free(typeP);
			typeP = NULL;
		}
	}

	
	TreeNode* varP = new TreeNode;
	if (varP == NULL)
	{
		syntaxError("内存溢出");
		Error = true;
	}
	else {
		for (int i = 0; i < 3; i++)
			varP->child[i] = NULL;
		varP->sibling = NULL;
		varP->nodekind = VarK;
		varP->lineno = token.lineShow;
		for (int i = 0; i < 10; i++)
		{
			varP->name[i] = "\0";
			varP->table[i] = NULL;
		}
	}

	if (varP != NULL)
	{
		varP->lineno = 0;
		TreeNode* tp2 = VarDec();
		if (tp2 != NULL)
			varP->child[0] = tp2;
		else
		{
			free(varP);
			varP = NULL;
		}
	}

	TreeNode* s = ProcDec();

	if (varP == NULL) 
		varP = s;

	if (typeP == NULL)
		pp = typeP = varP;

	//?typeP = varP, VarP = s
	if (typeP != varP)
	{
		typeP->sibling = varP;
		typeP = varP;
	}

	if (varP != s)
	{
		varP->sibling = s;
		varP = s;
	}

	return pp;
}

//？< typeDec > ::= ε | TypeDeclaration  
TreeNode* RecursiveDescentParsing::TypeDec(void)
{
	TreeNode* t = NULL;
	if (token.word.Lex == TYPE)
		t = TypeDeclaration();

	else if (token.word.Lex == VAR || token.word.Lex == PROCEDURE || token.word.Lex == BEGIN ){}

	else
	{
		ReadNextToken();
		syntaxError("意外符号");
	}
	return t;
}

TreeNode* RecursiveDescentParsing::TypeDeclaration(void)
{
	match(TYPE);
	TreeNode* t = TypeDecList();
	if (t == NULL)
		syntaxError("缺少类型声明");
	return t;
}

TreeNode* RecursiveDescentParsing::TypeDecList(void)
{
	TreeNode* t = new TreeNode;
	if (t == NULL)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) 
			t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = DecK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = NULL;
		}
	}

	if (t != NULL)
	{
		t->lineno = line0;
		TypeId(t);
		match(EQ);
		TypeName(t);
		match(SEMI);
		TreeNode* p = TypeDecMore();
		if (p != NULL)
			t->sibling = p;
	}
	return t;
}

//？< typeDecMore > ::=    ε | TypeDecList
TreeNode* RecursiveDescentParsing::TypeDecMore(void)
{
	TreeNode* t = NULL;
	if (token.word.Lex == VAR || token.word.Lex == PROCEDURE || token.word.Lex == BEGIN) {}
	else if (token.word.Lex == ID)
		t = TypeDecList();
	else
	{
		ReadNextToken();
		syntaxError("意外符号");
	}
	return t;
}

void RecursiveDescentParsing::TypeId(TreeNode* t)
{
	if (t != NULL)
	{
		int temp = t->idnum;
		if ((token.word.Lex == ID) && (t != NULL))
		{
			t->name[temp] = token.word.Sem;
			temp++;
		}
		t->idnum = temp;
		match(ID);
	}
}

//？< typeName > ::= baseType | structureType | id 
void RecursiveDescentParsing::TypeName(TreeNode* t)
{
	if (t != NULL)
	{
		if (token.word.Lex == INTEGER || token.word.Lex == CHAR1)
			BaseType(t);
		else if (token.word.Lex == ARRAY || token.word.Lex == RECORD)
			StructureType(t);
		else if (token.word.Lex == ID)
		{
			t->kind.dec = IdK;
			t->attr.type_name = token.word.Sem;
			match(ID);
		}
		else
		{
			ReadNextToken();
			syntaxError("意外符号");
		}
	}
}

//？< baseType > ::=  INTEGER | CHAR 
void RecursiveDescentParsing::BaseType(TreeNode* t)
{
	if (t != NULL)
	{
		if (token.word.Lex == INTEGER)
		{
			match(INTEGER);
			t->kind.dec = IntegerK;
		}
		else if (token.word.Lex == CHAR1)
		{
			match(CHAR1);
			t->kind.dec = CharK;
		}
		else
		{
			ReadNextToken();
			syntaxError("意外符号");
		}
	}
}

//？< structureType > ::=  arrayType | recType
void RecursiveDescentParsing::StructureType(TreeNode* t)
{
	if (t != NULL)
	{
		if (token.word.Lex == ARRAY)
			ArrayType(t);
		else if (token.word.Lex == RECORD)
		{
			t->kind.dec = RecordK;
			RecType(t);
		}
		else
		{
			ReadNextToken();
			syntaxError("意外符号");
		}
	}
}

void RecursiveDescentParsing::ArrayType(TreeNode* t)
{
	match(ARRAY);
	match(LMIDPAREN);
	if (token.word.Lex == INTC)
	{
		t->attr.ArrayAttr.low = atoi(token.word.Sem.c_str());
	}
	match(INTC);
	match(UNDERANGE);
	if (token.word.Lex == INTC)
	{
		t->attr.ArrayAttr.up = atoi(token.word.Sem.c_str());
	}
	match(INTC);
	match(RMIDPAREN);
	match(OF);
	BaseType(t);
	t->attr.ArrayAttr.childtype = t->kind.dec;
	t->kind.dec = ArrayK;
}

void RecursiveDescentParsing::RecType(TreeNode* t)
{
	TreeNode* p = NULL;
	match(RECORD);
	p = FieldDecList();
	if (p != NULL)
		t->child[0] = p;
	else
		syntaxError("记录内容缺失");
	match(END);
}

//?< fieldDecList > ::=   baseType idList ; fieldDecMore  | arrayType idList; fieldDecMore
TreeNode* RecursiveDescentParsing::FieldDecList(void)
{
	TreeNode* t = new TreeNode;
	if (t == NULL)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++)
			t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = DecK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = NULL;
		}
	}

	TreeNode* p = NULL;
	if (t != NULL)
	{
		t->lineno = line0;
		if (token.word.Lex == INTEGER || token.word.Lex == CHAR1)
		{
			BaseType(t);
			IdList(t);
			match(SEMI);
			p = FieldDecMore();
		}
		else if (token.word.Lex == ARRAY)
		{
			ArrayType(t);
			IdList(t);
			match(SEMI);
			p = FieldDecMore();
		}
		else
		{
			ReadNextToken();
			syntaxError("意外符号");
		}
		t->sibling = p;
	}
	return t;
}

//?< fieldDecMore > ::=  ε | fieldDecList
TreeNode* RecursiveDescentParsing::FieldDecMore(void)
{
	TreeNode* t = NULL;
	if (token.word.Lex == END) {}
	else if (token.word.Lex == INTEGER || token.word.Lex == CHAR1 || token.word.Lex == ARRAY)
		t = FieldDecList();
	else
	{
		ReadNextToken();
		syntaxError("意外符号");
	}
	return t;
}

void RecursiveDescentParsing::IdList(TreeNode* t)
{
	if (t != NULL)
	{
		if (token.word.Lex == ID)
		{
			t->name[t->idnum] = token.word.Sem;
			match(ID);
			t->idnum++;
		}
		IdMore(t);
	}
}

//?< idMore > ::=  ε |  , idList  
void RecursiveDescentParsing::IdMore(TreeNode* t)
{
	if (token.word.Lex == SEMI){}
	else if (token.word.Lex == COMMA)
	{
		match(COMMA);
		IdList(t);
	}
	else
	{
		ReadNextToken();
		syntaxError("意外符号");
	}
}

//?< varDec > ::=  ε |  varDeclaration  
TreeNode* RecursiveDescentParsing::VarDec(void)
{
	TreeNode* t = NULL;
	if (token.word.Lex == PROCEDURE || token.word.Lex == BEGIN){}
	else if (token.word.Lex == VAR)
		t = VarDeclaration();
	else
	{
		ReadNextToken();
		syntaxError("意外符号");
	}
	return t;
}

TreeNode* RecursiveDescentParsing::VarDeclaration(void)
{
	match(VAR);
	TreeNode* t = VarDecList();
	if (t == NULL)
		syntaxError("变量声明缺失");
	return t;
}

TreeNode* RecursiveDescentParsing::VarDecList(void)
{
	TreeNode* t = new TreeNode;
	if (t == NULL)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++)
			t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = DecK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = NULL;
		}
	}

	TreeNode* p = NULL;
	if (t != NULL)
	{
		t->lineno = line0;
		TypeName(t);
		VarIdList(t);
		match(SEMI);
		p = VarDecMore();
		t->sibling = p;
	}
	return t;
}

//？< varDecMore > ::=  ε |  varDecList 
TreeNode* RecursiveDescentParsing::VarDecMore(void)
{
	TreeNode* t = NULL;
	if (token.word.Lex == PROCEDURE || token.word.Lex == BEGIN){}
	else if (token.word.Lex == INTEGER || token.word.Lex == CHAR1 || token.word.Lex == ARRAY ||
			 token.word.Lex == RECORD || token.word.Lex == ID)
		t = VarDecList();
	else
	{
		ReadNextToken();
		syntaxError("意外符号");
	}
	return t;
}

void RecursiveDescentParsing::VarIdList(TreeNode* t)
{
	if (token.word.Lex == ID)
	{
		t->name[(t->idnum)] = token.word.Sem;
		match(ID);
		t->idnum++;
	}
	else
	{
		syntaxError("a varid is expected here!");
		ReadNextToken();
	}
	VarIdMore(t);
}

//？< varIdMore > ::=  ε |  , varIdList 
void RecursiveDescentParsing::VarIdMore(TreeNode* t)
{
	if (token.word.Lex == SEMI){}
	else if (token.word.Lex == COMMA)
	{
		match(COMMA);
		VarIdList(t);
	}
	else
	{
		ReadNextToken();
		syntaxError("意外符号");
	}
}

//？< procDec > ::=  ε |  procDeclaration
TreeNode* RecursiveDescentParsing::ProcDec(void)
{
	TreeNode* t = NULL;
	if (token.word.Lex == BEGIN){}
	else if (token.word.Lex == PROCEDURE)
		t = ProcDeclaration();
	else
	{
		ReadNextToken();
		syntaxError("意外符号");
	}
	return t;
}

TreeNode* RecursiveDescentParsing::ProcDeclaration(void)
{
	TreeNode* t = new TreeNode;
	if (t == NULL)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = ProcDecK;
		t->lineno = token.lineShow;
		t->idnum = 0;

		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = NULL;
		}
	}

	match(PROCEDURE);
	if (t != NULL)
	{
		t->lineno = line0;
		if (token.word.Lex == ID)
		{
			t->name[0] = token.word.Sem;
			(t->idnum)++;
			match(ID);
		}
		match(LPAREN);
		ParamList(t);
		match(RPAREN);
		match(SEMI);
		t->child[1] = ProcDecPart();
		t->child[2] = ProcBody();
		t->sibling = ProcDec();
	}
	return t;
}

void RecursiveDescentParsing::ParamList(TreeNode* t)
{
	TreeNode* p = NULL;
	if (token.word.Lex == RPAREN){}
	else if (token.word.Lex == INTEGER || token.word.Lex == CHAR1 || token.word.Lex == ARRAY ||
			 token.word.Lex == RECORD || token.word.Lex == ID || token.word.Lex == VAR)
	{
		p = ParamDecList();
		t->child[0] = p;
	}
	else
	{
		ReadNextToken();
		syntaxError("意外符号");
	}
}

TreeNode* RecursiveDescentParsing::ParamDecList(void)
{
	TreeNode * t = Param();
	TreeNode * p = ParamMore();
	if (p!=NULL)
	{
		t->sibling = p;
	}
	return t;
}

//？< paramMore > ::=  ε | ; paramDecList 
TreeNode* RecursiveDescentParsing::ParamMore(void)
{
	TreeNode* t = NULL;
	if (token.word.Lex == RPAREN){}
	else if (token.word.Lex == SEMI)
	{
		match(SEMI);
		t = ParamDecList();
		if (t == NULL)
			syntaxError("参数缺失");
	}
	else
	{
		ReadNextToken();
		syntaxError("意外符号");
	}
	return t;
}

//？< param > ::=  typeName formList | VAR typeName formList
TreeNode* RecursiveDescentParsing::Param(void)
{
	TreeNode* t = new TreeNode;
	if (t == NULL)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++)
			t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = DecK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = NULL;
		}
	}

	if (t != NULL)
	{
		t->lineno = line0;
		if (token.word.Lex == INTEGER || token.word.Lex == INTEGER || token.word.Lex == INTEGER || 
			token.word.Lex == INTEGER || token.word.Lex == INTEGER )
		{
			t->attr.ProcAttr.paramt = valparamType;
			TypeName(t);
			FormList(t);
		}
		else if (token.word.Lex == VAR)
		{
			match(VAR);
			t->attr.ProcAttr.paramt = varparamType;
			TypeName(t);
			FormList(t);
		}
		else
		{
			ReadNextToken();
			syntaxError("意外符号");
		}
	}
	return t;
}

void RecursiveDescentParsing::FormList(TreeNode* t)
{
	if (t != NULL)
	{
		if (token.word.Lex == ID)
		{
			t->name[t->idnum] = token.word.Sem;
			t->idnum++;
			match(ID);
		}
		FidMore(t);
	}
}

void RecursiveDescentParsing::FidMore(TreeNode* t)
{
	if (token.word.Lex == SEMI || token.word.Lex == RPAREN){}
	else if (token.word.Lex == COMMA)
	{
		match(COMMA);
		FormList(t);
	}
	else
	{
		ReadNextToken();
		syntaxError("意外符号");
	}
}

TreeNode* RecursiveDescentParsing::ProcDecPart(void)
{
	//newDecANode(typeK);
	TreeNode* typeP = new TreeNode;
	if (typeP == NULL)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) 
			typeP->child[i] = NULL;
		typeP->sibling = NULL;
		typeP->nodekind = TypeK;
		typeP->lineno = token.lineShow;
		for (int i = 0; i < 10; i++)
		{
			typeP->name[i] = "\0";
			typeP->table[i] = NULL;
		}
	}

	TreeNode* pp = typeP;
	if (typeP != NULL)
	{
		typeP->lineno = 0;
		TreeNode* tp1 = TypeDec();
		if (tp1 != NULL)
			typeP->child[0] = tp1;
		else
		{
			free(typeP);
			typeP = NULL;
		}
	}

	//newDecANode(VarK);
	TreeNode* varP = new TreeNode;
	if (varP == NULL)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++)
			varP->child[i] = NULL;
		varP->sibling = NULL;
		varP->nodekind = VarK;
		varP->lineno = token.lineShow;
		for (int i = 0; i < 10; i++)
		{
			varP->name[i] = "\0";
			varP->table[i] = NULL;
		}
	}

	if (varP != NULL)
	{
		varP->lineno = 0;
		TreeNode* tp2 = VarDec();
		if (tp2 != NULL)
			varP->child[0] = tp2;
		else
		{
			free(varP);
			varP = NULL;
		}
	}
	TreeNode* s = ProcDec();
	if (s == NULL) {}

	if (varP == NULL) 
		varP = s;

	if (typeP == NULL) 
		pp = typeP = varP;

	if (typeP != varP)
	{
		typeP->sibling = varP;
		typeP = varP;
	}
	if (varP != s)
	{
		varP->sibling = s;
		varP = s;
	}
	return pp;
}

TreeNode* RecursiveDescentParsing::ProcBody(void)
{
	TreeNode* t = ProgramBody();
	if (t == NULL)
		syntaxError("程序体缺失");
	return t;
}

TreeNode* RecursiveDescentParsing::ProgramBody(void)
{
	//newStmlNode();
	TreeNode* t = new TreeNode;
	if (t == NULL)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = StmLK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = NULL;
		}
	}

	match(BEGIN);
	if (t != NULL)
	{
		t->lineno = 0;
		t->child[0] = StmList();
	}
	match(END);
	return t;
}

TreeNode* RecursiveDescentParsing::StmList(void)
{
	TreeNode* t = Stm();
	TreeNode* p = StmMore();
	if (t != NULL)
		if (p != NULL)
			t->sibling = p;
	return t;
}

//?< stmMore > ::=   ε |  ; stmList  ???END,ENDWH,FI,ELSE
TreeNode* RecursiveDescentParsing::StmMore(void)
{
	TreeNode* t = NULL;
	if (token.word.Lex == END || token.word.Lex == ENDWH){}
	else if (token.word.Lex == SEMI)
	{
		match(SEMI);
		t = StmList();
	}
	else
	{
		ReadNextToken();
		syntaxError("意外符号");
	}
	return t;
}

//? < stm > ::=   conditionalStm   {IF}      | loopStm          {WHILE}
//	            | inputStm         {READ}    | outputStm        {WRITE}
//	            | returnStm        {RETURN}  | id  assCall      {id}
TreeNode* RecursiveDescentParsing::Stm(void)
{
	TreeNode* t = NULL;
	if (token.word.Lex == IF)
		t = ConditionalStm();
	else if (token.word.Lex == WHILE)
		t = LoopStm();
	else if (token.word.Lex == READ)
		t = InputStm();
	else if (token.word.Lex == WRITE)
		t = OutputStm();
	else if (token.word.Lex == RETURN)
		t = ReturnStm();
	else if (token.word.Lex == ID)
	{
		temp_name = token.word.Sem;
		match(ID);
		t = AssCall();
	}
	else
	{
		ReadNextToken();
		syntaxError("意外符号");
	}
	return t;
}

//？< assCall > ::=   assignmentRest  {:=,LMIDPAREN,DOT}  | callStmRest  {(}   ???LMIDPAREN,DOT
TreeNode* RecursiveDescentParsing::AssCall(void)
{
	TreeNode* t = NULL;
	if (token.word.Lex == ASSIGN)
		t = AssignmentRest();
	else if (token.word.Lex == LPAREN)
		t = CallStmRest();
	else
	{
		ReadNextToken();
		syntaxError("意外符号");
	}
	return t;
}

TreeNode* RecursiveDescentParsing::AssignmentRest(void)
{
	//newStmtNode()
	TreeNode* t = new TreeNode;
	if (t == NULL)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = StmtK;
		t->kind.stmt = AssignK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = NULL;
		}
	}

	if (t != NULL)
	{
		t->lineno = line0;

		//newExpNode
		TreeNode* child1 = new TreeNode;
		if (child1 == NULL)
			syntaxError("内存泄露");
		else {
			for (int i = 0; i < 3; i++)
				child1->child[i] = NULL;
			child1->sibling = NULL;
			child1->nodekind = ExpK;
			child1->kind.exp = VariK;
			child1->lineno = token.lineShow;
			child1->attr.ExpAttr.varkind = IdV;
			child1->attr.ExpAttr.type = Void;
			for (int i = 0; i < 10; i++)
			{
				child1->name[i] = "\0";
				child1->table[i] = NULL;
			}
		}

		if (child1 != NULL)
		{
			child1->lineno = line0;
			child1->name[0] = temp_name;
			(child1->idnum)++;
			VariMore(child1);
			t->child[0] = child1;
		}
		match(ASSIGN);
		t->child[1] = Exp();
	}
	return t;
}

TreeNode* RecursiveDescentParsing::ConditionalStm(void)
{
	//newStmtNode()
	TreeNode* t = new TreeNode;
	if (t == NULL)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = StmtK;
		t->kind.stmt = IfK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = NULL;
		}
	}

	match(IF);
	if (t != NULL)
	{
		t->lineno = line0;
		t->child[0] = Exp();
	}
	match(THEN);
	if (t != NULL)  t->child[1] = StmList();
	if (token.word.Lex == ELSE)
	{
		match(ELSE);
		if (t != NULL)
			t->child[2] = StmList();
	}
	match(FI);
	return t;
}

TreeNode* RecursiveDescentParsing::LoopStm(void)
{
	//newStmtNode()
	TreeNode* t = new TreeNode;
	if (t == NULL)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = StmtK;
		t->kind.stmt = WhileK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = NULL;
		}
	}

	match(WHILE);
	if (t != NULL)
	{
		t->lineno = line0;
		t->child[0] = Exp();
		match(DO);
		t->child[1] = StmList();
		match(ENDWH);
	}
	return t;
}

TreeNode* RecursiveDescentParsing::InputStm(void)
{
	//newStmtNode()
	TreeNode* t = new TreeNode;
	if (t == NULL)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = StmtK;
		t->kind.stmt = ReadK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = NULL;
		}
	}

	match(READ);
	match(LPAREN);
	if ((t != NULL) && (token.word.Lex == ID))
	{
		t->lineno = line0;
		t->name[0] = token.word.Sem;
		t->idnum++;
	}
	match(ID);
	match(RPAREN);
	return t;
}

TreeNode* RecursiveDescentParsing::OutputStm(void)
{
	//newStmtNode()
	TreeNode* t = new TreeNode;
	if (t == NULL)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = StmtK;
		t->kind.stmt = WriteK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = NULL;
		}
	}

	match(WRITE);
	match(LPAREN);
	if (t != NULL)
	{
		t->lineno = line0;
		t->child[0] = Exp();
	}
	match(RPAREN);
	return t;
}

TreeNode* RecursiveDescentParsing::ReturnStm(void)
{
	//newStmtNode()
	TreeNode* t = new TreeNode;
	if (t == NULL)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = StmtK;
		t->kind.stmt = ReturnK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = NULL;
		}
	}

	match(RETURN);
	if (t != NULL)
		t->lineno = line0;
	return t;
}

TreeNode* RecursiveDescentParsing::CallStmRest(void)
{
	//newStmtNode()
	TreeNode* t = new TreeNode;
	if (t == NULL)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = StmtK;
		t->kind.stmt = CallK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = NULL;
		}
	}

	match(LPAREN);
	/*函数调用时，其子节点指向实参*/
	if (t != NULL)
	{
		t->lineno = line0;

		/*函数名的结点也用表达式类型结点*/
		TreeNode* child0 = new TreeNode;
		if (child0 == NULL)
			syntaxError("内存溢出");
		else {
			for (int i = 0; i < 3; i++) 
				child0->child[i] = NULL;
			child0->sibling = NULL;
			child0->nodekind = ExpK;
			child0->kind.exp = VariK;
			child0->lineno = token.lineShow;
			child0->attr.ExpAttr.varkind = IdV;
			child0->attr.ExpAttr.type = Void;
			for (int i = 0; i < 10; i++)
			{
				child0->name[i] = "\0";
				child0->table[i] = NULL;
			}
		}

		if (child0 != NULL)
		{
			child0->lineno = line0;
			child0->name[0] = temp_name;
			(child0->idnum)++;
			t->child[0] = child0;
		}
		t->child[1] = ActParamList();
	}
	match(RPAREN);
	return t;
}

//?< actParamList > ::=     ε |  exp actParamMore
TreeNode* RecursiveDescentParsing::ActParamList(void)
{
	TreeNode* t = NULL;
	if (token.word.Lex == RPAREN) {}
	else if (token.word.Lex == ID || token.word.Lex == INTC)
	{
		t = Exp();
		if (t != NULL)
			t->sibling = ActParamMore();
	}
	else
	{
		ReadNextToken();
		syntaxError("意外符号");
	}
	return t;
}

//?< actParamMore > ::=     ε |  , actParamList 
TreeNode* RecursiveDescentParsing::ActParamMore(void)
{
	TreeNode* t = NULL;
	if (token.word.Lex == RPAREN){}
	else if (token.word.Lex == COMMA)
	{
		match(COMMA);
		t = ActParamList();
	}
	else
	{
		ReadNextToken();
		syntaxError("意外符号");
	}
	return t;
}

TreeNode* RecursiveDescentParsing::Exp(void)
{
	TreeNode* t = Simple_exp();
	if ((token.word.Lex == LT) || (token.word.Lex == EQ))
	{
		//newExpNode()
		TreeNode* p = new TreeNode;
		if (p == NULL)
			syntaxError("内存泄露");
		else {
			for (int i = 0; i < 3; i++)
				p->child[i] = NULL;
			p->sibling = NULL;
			p->nodekind = ExpK;
			p->kind.exp = OpK;
			p->lineno = token.lineShow;
			p->attr.ExpAttr.varkind = IdV;
			p->attr.ExpAttr.type = Void;
			for (int i = 0; i < 10; i++)
			{
				p->name[i] = "\0";
				p->table[i] = NULL;
			}
		}

		if (p != NULL)
		{
			p->lineno = line0;
			p->child[0] = t;
			p->attr.ExpAttr.op = token.word.Lex;
			t = p;
		}
		match(token.word.Lex);
		if (t != NULL)
			t->child[1] = Simple_exp();
	}
	return t;
}

//？< simple_exp > ::= term | PlusOp term
TreeNode* RecursiveDescentParsing::Simple_exp(void)
{
	TreeNode* t = Term();
	while ((token.word.Lex == PLUS) || (token.word.Lex == MINUS))
	{
		//newExpNode()
		TreeNode* p = new TreeNode;
		if (p == NULL)
			syntaxError("内存泄露");
		else {
			for (int i = 0; i < 3; i++)
				p->child[i] = NULL;
			p->sibling = NULL;
			p->nodekind = ExpK;
			p->kind.exp = OpK;
			p->lineno = token.lineShow;
			p->attr.ExpAttr.varkind = IdV;
			p->attr.ExpAttr.type = Void;
			for (int i = 0; i < 10; i++)
			{
				p->name[i] = "\0";
				p->table[i] = NULL;
			}
		}

		if (p != NULL)
		{
			p->lineno = line0;
			p->child[0] = t;
			p->attr.ExpAttr.op = token.word.Lex;
			t = p;
			match(token.word.Lex);
			t->child[1] = Term();
		}
	}
	return t;
}

TreeNode* RecursiveDescentParsing::Term(void)
{
	TreeNode* t = Factor();
	while ((token.word.Lex == TIMES) || (token.word.Lex == OVER))

	{
		//newExpNode()
		TreeNode* p = new TreeNode;
		if (p == NULL)
			syntaxError("内存泄露");
		else {
			for (int i = 0; i < 3; i++)
				p->child[i] = NULL;
			p->sibling = NULL;
			p->nodekind = ExpK;
			p->kind.exp = OpK;
			p->lineno = token.lineShow;
			p->attr.ExpAttr.varkind = IdV;
			p->attr.ExpAttr.type = Void;
			for (int i = 0; i < 10; i++)
			{
				p->name[i] = "\0";
				p->table[i] = NULL;
			}
		}

		if (p != NULL)
		{
			p->lineno = line0;
			p->child[0] = t;
			p->attr.ExpAttr.op = token.word.Lex;
			t = p;
		}

		match(token.word.Lex);
		p->child[1] = Factor();

	}
	return t;
}

TreeNode* RecursiveDescentParsing::Factor(void)
{
	TreeNode* t = NULL;
	if (token.word.Lex == INTC)
	{
		//newExpNode()
		TreeNode* t = new TreeNode;
		if (t == NULL)
			syntaxError("内存泄露");
		else {
			for (int i = 0; i < 3; i++)
				t->child[i] = NULL;
			t->sibling = NULL;
			t->nodekind = ExpK;
			t->kind.exp = ConstK;
			t->lineno = token.lineShow;
			t->attr.ExpAttr.varkind = IdV;
			t->attr.ExpAttr.type = Void;
			for (int i = 0; i < 10; i++)
			{
				t->name[i] = "\0";
				t->table[i] = NULL;
			}
		}

		if ((t != NULL) && (token.word.Lex == INTC))
		{
			t->lineno = line0;
			t->attr.ExpAttr.val = atoi(token.word.Sem.c_str());
		}
		match(INTC);
	}
	else if (token.word.Lex == ID)
		t = Variable();
	else if (token.word.Lex == LPAREN)
	{
		match(LPAREN);
		t = Exp();
		match(RPAREN);
	}
	else
	{
		ReadNextToken();
		syntaxError("意外符号");
	}
	return t;
}

TreeNode* RecursiveDescentParsing::Variable(void)
{
	//newExpNode()
	TreeNode* t = new TreeNode;
	if (t == NULL)
		syntaxError("内存泄露");
	else {
		for (int i = 0; i < 3; i++)
			t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = ExpK;
		t->kind.exp = VariK;
		t->lineno = token.lineShow;
		t->attr.ExpAttr.varkind = IdV;
		t->attr.ExpAttr.type = Void;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = NULL;
		}
	}

	if ((t != NULL) && (token.word.Lex == ID))
	{
		t->lineno = line0;
		t->name[0] = token.word.Sem;
		(t->idnum)++;
	}
	match(ID);
	VariMore(t);
	return t;
}

//？variMore   ::=  ε                             			
//                | [exp]                          
//                | . fieldvar                 
void RecursiveDescentParsing::VariMore(TreeNode* t)
{
	if (token.word.Lex == ASSIGN || token.word.Lex == TIMES || token.word.Lex == EQ ||
		token.word.Lex == LT || token.word.Lex == PLUS || token.word.Lex == MINUS ||
		token.word.Lex == OVER || token.word.Lex == RPAREN || token.word.Lex == RMIDPAREN ||
		token.word.Lex == SEMI || token.word.Lex == COMMA || token.word.Lex == THEN || 
		token.word.Lex == ELSE || token.word.Lex == FI || token.word.Lex == DO || 
		token.word.Lex == ENDWH || token.word.Lex == END){}
	else if (token.word.Lex == LMIDPAREN)
	{
		match(LMIDPAREN);
		t->child[0] = Exp();
		t->attr.ExpAttr.varkind = ArrayMembV;
		t->child[0]->attr.ExpAttr.varkind = IdV;
		match(RMIDPAREN);
	}
	else if (token.word.Lex == DOT)
	{
		match(DOT);
		t->child[0] = Fieldvar();
		t->attr.ExpAttr.varkind = FieldMembV;
		t->child[0]->attr.ExpAttr.varkind = IdV;
	}
	else
	{
		ReadNextToken();
		syntaxError("意外符号");
	}
}

TreeNode* RecursiveDescentParsing::Fieldvar(void)
{
	//newExpNode
	TreeNode* t = new TreeNode;
	if (t == NULL)
		syntaxError("内存泄露");
	else {
		for (int i = 0; i < 3; i++)
			t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = ExpK;
		t->kind.exp = VariK;
		t->lineno = token.lineShow;
		t->attr.ExpAttr.varkind = IdV;
		t->attr.ExpAttr.type = Void;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = NULL;
		}
	}

	if ((t != NULL) && (token.word.Lex == ID))
	{
		t->lineno = line0;
		t->name[0] = token.word.Sem;
		t->idnum++;
	}
	match(ID);
	FieldvarMore(t);
	return t;
}

//fieldvarMore   ::=  ε  | [exp]           
void RecursiveDescentParsing::FieldvarMore(TreeNode* t)
{
	if (token.word.Lex == ASSIGN || token.word.Lex == TIMES || token.word.Lex == EQ ||
		token.word.Lex == LT || token.word.Lex == PLUS || token.word.Lex == MINUS ||
		token.word.Lex == OVER || token.word.Lex == RPAREN || token.word.Lex == SEMI || 
		token.word.Lex == COMMA || token.word.Lex == THEN || token.word.Lex == ELSE || 
		token.word.Lex == FI || token.word.Lex == DO || token.word.Lex == ENDWH || token.word.Lex == END){}
	else if (token.word.Lex == LMIDPAREN)
	{
		match(LMIDPAREN);
		t->child[0] = Exp();
		t->child[0]->attr.ExpAttr.varkind = ArrayMembV;
		match(RMIDPAREN);
	}
	else
	{
		ReadNextToken();
		syntaxError("意外符号");
	}
}


