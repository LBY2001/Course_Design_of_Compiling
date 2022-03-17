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
int tokenNum;	//读到第几个token序列了
bool Error;
Token token;	//正在判断的token序列
int line0;		//代码行数

void RecursiveDescentParsing::initial()
{
	Error = false;
	tokenNum = 0;
	line0 = 0;
	fopen_s(&flisting, "parseError.txt", "w");
}

void RecursiveDescentParsing::ReadNextToken()
{
	FILE* fp;
	fopen_s(&fp, "tokenList.txt", "rb");
	if (fp == NULL)
	{
		printf("tokenList打开失败!\n");
		Error = true;
		exit(0);
	}
	fseek(fp, tokenNum * (sizeof(Token) + 2 * sizeof(' ') + sizeof('\n')), 0);
	fscanf_s(fp,"%d%d%s",&token.lineShow, &token.word.Lex, token.word.Sem);
	char* temp;
	fread(&temp, sizeof('\n'), 1, fp);
	tokenNum++;
	fclose(fp);
}

void RecursiveDescentParsing::syntaxError(string errorMessage)
{
	fprintf(flisting, "Syntax error, line %d : %s\n\n", token.lineShow, errorMessage);
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
		syntaxError("not match error ");
		ReadNextToken();
	}
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
	TreeNode* root = (TreeNode*)malloc(sizeof(TreeNode));
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
			root->name[i] = "\0";
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
	TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
	if (t == NULL)
	{
		syntaxError("内存溢出");
		Error = true;
	}
	else {
		for (int i = 0; i < 3; i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = PheadK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
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
	TreeNode* typeP = (TreeNode*)malloc(sizeof(TreeNode));
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

	
	TreeNode* varP = (TreeNode*)malloc(sizeof(TreeNode));
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

//？predict集合
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

TreeNode* RecursiveDescentParsing::TypeDecList(void)
{
	TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
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

//？predict集合
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

//？predict集合
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

//？predict集合
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


