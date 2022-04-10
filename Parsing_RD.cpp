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
#include <graphics.h>
#include"Parsing_RD.h"

FILE* fin;
FILE* flisting;
int tokenNum;		//读到第几个token序列了
Token token;		//正在判断的token序列
int line0;			//在代码中的行数
string temp_name;	//临时存变量名
ofstream textfile;	//输出文件
FILE* fp;			//读入文件
int indentation;	//缩进
ofstream treeFile;	//树形文件
FILE* listing;
bool Error;
bool hasTypeK;

//语法树可视化使用变量
int circleX, circleY, circleR;	//语法树图形化节点坐标x, y, 半径r
TCHAR temp[100];
int lastX;
int lastY;
NodeKind lastNode;
bool lastIsDeck;
int procNum;			//代码中声明的数量
int stmlkNum;			//代码段数量

//类型哈希表
unordered_map<int, string> ha2 =
{
	{0, "ENDFILE1"},	{1, "ERROR1"},		{2, "PROGRAM"},		{3,"PROCEDURE"},
	{4, "TYPE"},		{5, "VAR"},			{6, "IF"},			{7, "THEN"},
	{8, "ELSE"},		{9, "FI"},			{10, "WHILE"},		{11, "DO"},
	{12, "ENDWH"},		{13, "BEGIN"},		{14, "END"},		{15, "READ"},
	{16, "WRITE"},		{11, "ARRAY"},		{12, "OF"},			{13, "RECORD"},
	{20, "RETURN"},		{21, "INTEGER"},	{22, "CHAR1"},		{23, "ID"},
	{24, "INTC"},		{25, "CHARC"},		{26, "ASSIGN"},		{27, "EQ"},
	{28, "LT"},			{29, "PLUS"},		{30, "MINUS"},		{31, "TIMES"},
	{32, "OVER"},		{33, "LPAREN"},		{34, "RPAREN"},		{35, "DOT"},
	{36, "COLON"},		{37, "SEMI"},		{38, "COMMA"},		{39, "LMIDPAREN"},
	{40, "RMIDPAREN"},	{41, "UNDERANGE"}
};

void RecursiveDescentParsing::initial()
{
	Error = false;
	tokenNum = 0;
	line0 = 0;
	//fopen_s(&flisting, "parseError.txt", "w");
	textfile.open("parseError.txt");
	treeFile.open("SyntaxTree.txt");
	fopen_s(&fp, "tokenList.txt", "rb");
	procNum = 0;
	indentation = 0;
	stmlkNum = 2;	//值为2，但代码段为1。在函数中减为1
	hasTypeK = false;
}

void RecursiveDescentParsing::ReadNextToken()
{
	//FILE* fp;
	//fopen_s(&fp, "tokenList.txt", "rb");
	if (fp == nullptr)
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
	textfile << "语法错误, line " << token.lineShow << " : " << errorMessage << endl;
	//fprintf(flisting, "Syntax error, line %d : %s\n\n", token.lineShow, errorMessage);
	Error = true;
}

void RecursiveDescentParsing::match(LexType lt)
{
	if (token.word.Lex == lt)
	{
		//对EOF未进行处理
		ReadNextToken();
		line0 = token.lineShow;
	}
	else
	{
		string tempError;
		tempError = "符号\"" + token.word.Sem + "\"错误，也许缺失符号\"" + ha2.at(lt) + "\"";
		syntaxError(tempError);
		//ReadNextToken();
	}
}

void RecursiveDescentParsing::fileClose()
{
	fclose(fp);
	textfile.close();
	treeFile.close();
}

TreeNode* RecursiveDescentParsing::parse(void)
{
	ReadNextToken();

	TreeNode* t = nullptr;
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
	if (root == nullptr)
	{
		syntaxError("内存溢出");
		Error = true;
		exit(0);
	}
	else {
		for (int i = 0; i < 3; i++)
			root->child[i] = nullptr;
		root->sibling = nullptr;
		root->nodekind = ProK;
		root->lineno = 0;
		for (int i = 0; i < 10; i++)
		{
			root->name[i] = "";
			root->table[i] = nullptr;
		}
	}


	if (root != nullptr)
	{
		root->lineno = 0;
		if (t != nullptr) 
			root->child[0] = t;
		else 
			syntaxError("缺少程序头");
		if (q != nullptr) root->child[1] = q;
		if (s != nullptr) root->child[2] = s;
		else 
			syntaxError("缺少程序体");
	}
	match(DOT);

	return root;
}

TreeNode* RecursiveDescentParsing::ProgramHead(void)
{
	TreeNode* t = new TreeNode;
	if (t == nullptr)
	{
		syntaxError("内存溢出");
		Error = true;
	}
	else {
		for (int i = 0; i < 3; i++) 
			t->child[i] = nullptr;
		t->sibling = nullptr;
		t->nodekind = PheadK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "";
			t->table[i] = nullptr;
		}
	}
	match(PROGRAM);
	if ((t != nullptr) && (token.word.Lex == ID))
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
	if (typeP == nullptr)
	{
		syntaxError("内存溢出");
		Error = true;
	}
	else {
		for (int i = 0; i < 3; i++) 
			typeP->child[i] = nullptr;
		typeP->sibling = nullptr;
		typeP->nodekind = TypeK;
		typeP->lineno = token.lineShow;
		for (int i = 0; i < 10; i++)
		{
			typeP->name[i] = "\0";
			typeP->table[i] = nullptr;
		}
	}

	TreeNode* pp = typeP;
	if (typeP != nullptr)
	{
		typeP->lineno = 0;
		TreeNode* tp1 = TypeDec();
		if (tp1 != nullptr)
			typeP->child[0] = tp1;
		else
		{
			free(typeP);
			typeP = nullptr;
		}
	}

	
	TreeNode* varP = new TreeNode;
	if (varP == nullptr)
	{
		syntaxError("内存溢出");
		Error = true;
	}
	else {
		for (int i = 0; i < 3; i++)
			varP->child[i] = nullptr;
		varP->sibling = nullptr;
		varP->nodekind = VarK;
		varP->lineno = token.lineShow;
		for (int i = 0; i < 10; i++)
		{
			varP->name[i] = "\0";
			varP->table[i] = nullptr;
		}
	}

	if (varP != nullptr)
	{
		varP->lineno = 0;
		TreeNode* tp2 = VarDec();
		if (tp2 != nullptr)
			varP->child[0] = tp2;
		else
		{
			free(varP);
			varP = nullptr;
		}
	}

	TreeNode* s = ProcDec();

	if (varP == nullptr) 
		varP = s;

	if (typeP == nullptr)
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

//?< typeDec > ::= ε | TypeDeclaration  
TreeNode* RecursiveDescentParsing::TypeDec(void)
{
	TreeNode* t = nullptr;
	if (token.word.Lex == TYPE)
		t = TypeDeclaration();

	else if (token.word.Lex == VAR || token.word.Lex == PROCEDURE || token.word.Lex == BEGIN ){}

	else
	{
		string tempError = "意外符号\"" + token.word.Sem + "\"" + "，或许缺少符号“TYPE” / “VAR” / “PROCEDURE” / “BEGIN”";
		syntaxError(tempError);
		//ReadNextToken();
	}
	return t;
}

TreeNode* RecursiveDescentParsing::TypeDeclaration(void)
{
	match(TYPE);
	TreeNode* t = TypeDecList();
	if (t == nullptr)
		syntaxError("缺少类型声明");
	return t;
}

TreeNode* RecursiveDescentParsing::TypeDecList(void)
{
	//newDecNode()
	TreeNode* t = new TreeNode;
	if (t == nullptr)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) 
			t->child[i] = nullptr;
		t->sibling = nullptr;
		t->nodekind = DecK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = nullptr;
		}
	}

	if (t != nullptr)
	{
		t->lineno = line0;
		TypeId(t);
		match(EQ);
		TypeName(t);
		match(SEMI);
		
		/*
		//结构体变量数目放在第一个变量的idnum里
		//！！！！！！
		if (t->kind.dec == RecordK && t->child[0] != NULL)
		{
			TreeNode* temp = t->child[0];
			int tempIdnum = 0;
			while (temp != nullptr)
			{
				tempIdnum += temp->idnum;
				temp = temp->sibling;
			}
			t->child[0]->idnum = tempIdnum;
		}*/

		TreeNode* p = TypeDecMore();
		if (p != nullptr)
			t->sibling = p;
	}
	return t;
}

//?< typeDecMore > ::=    ε | TypeDecList
TreeNode* RecursiveDescentParsing::TypeDecMore(void)
{
	TreeNode* t = nullptr;
	if (token.word.Lex == VAR || token.word.Lex == PROCEDURE || token.word.Lex == BEGIN) {}
	else if (token.word.Lex == ID)
		t = TypeDecList();
	else
	{
		string tempError = "意外符号\"" + token.word.Sem + "\"" + "，或许缺少符号“变量名” / “VAR” / “PROCEDURE” / “BEGIN”";
		syntaxError(tempError);
		//ReadNextToken();
	}
	return t;
}

void RecursiveDescentParsing::TypeId(TreeNode* t)
{
	if (t != nullptr)
	{
		int temp = t->idnum;
		if ((token.word.Lex == ID) && (t != nullptr))
		{
			t->name[temp] = token.word.Sem;
			temp++;
		}
		t->idnum = temp;
		match(ID);
	}
}

//?< typeName > ::= baseType | structureType | id 
void RecursiveDescentParsing::TypeName(TreeNode* t)
{
	if (t != nullptr)
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
			string tempError = "意外符号\"" + token.word.Sem + "\"" + "，或许缺少符号“变量名” / “INTEGER” / “CHAR1” / “ARRAY” / “RECORD”";
			syntaxError(tempError);
			//ReadNextToken();
		}
	}
}

//?< baseType > ::=  INTEGER | CHAR 
void RecursiveDescentParsing::BaseType(TreeNode* t)
{
	if (t != nullptr)
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
			string tempError = "意外符号\"" + token.word.Sem + "\"" + "，或许缺少符号“INTEGER” / “CHAR1”";
			syntaxError(tempError);
			//ReadNextToken();
		}
	}
}

//?< structureType > ::=  arrayType | recType
void RecursiveDescentParsing::StructureType(TreeNode* t)
{
	if (t != nullptr)
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
			string tempError = "意外符号\"" + token.word.Sem + "\"" + "，或许缺少符号“ARRAY” / “RECCORD”";
			syntaxError(tempError);
			//ReadNextToken();
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
	TreeNode* p = nullptr;
	match(RECORD);
	p = FieldDecList();
	if (p != nullptr)
		t->child[0] = p;
	else
		syntaxError("记录内容缺失");
	match(END);
}

//?< fieldDecList > ::=   baseType idList ; fieldDecMore  | arrayType idList; fieldDecMore
TreeNode* RecursiveDescentParsing::FieldDecList(void)
{
	TreeNode* t = new TreeNode;
	if (t == nullptr)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++)
			t->child[i] = nullptr;
		t->sibling = nullptr;
		t->nodekind = DecK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = nullptr;
		}
	}

	TreeNode* p = nullptr;
	if (t != nullptr)
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
			string tempError = "意外符号\"" + token.word.Sem + "\"" + "，或许缺少符号“INTEGER” / “CHAR1” / “ARRAY”";
			syntaxError(tempError);
			//ReadNextToken();
		}
		t->sibling = p;
	}
	return t;
}

//?< fieldDecMore > ::=  ε | fieldDecList
TreeNode* RecursiveDescentParsing::FieldDecMore(void)
{
	TreeNode* t = nullptr;
	if (token.word.Lex == END) {}
	else if (token.word.Lex == INTEGER || token.word.Lex == CHAR1 || token.word.Lex == ARRAY)
		t = FieldDecList();
	else
	{
		string tempError = "意外符号\"" + token.word.Sem + "\"" + "，或许缺少符号“END” / “INTEGER” / “CHAR1” / “ARRAY”";
		syntaxError(tempError);
		//ReadNextToken();
	}
	return t;
}

void RecursiveDescentParsing::IdList(TreeNode* t)
{
	if (t != nullptr)
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
		string tempError = "意外符号\"" + token.word.Sem + "\"" + "，或许缺少符号“ ；”/ “，”";
		syntaxError(tempError);
		//ReadNextToken();
	}
}

//?< varDec > ::=  ε |  varDeclaration  
TreeNode* RecursiveDescentParsing::VarDec(void)
{
	TreeNode* t = nullptr;
	if (token.word.Lex == PROCEDURE || token.word.Lex == BEGIN){}
	else if (token.word.Lex == VAR)
		t = VarDeclaration();
	else
	{
		string tempError = "意外符号\"" + token.word.Sem + "\"" + "，或许缺少符号“PROCEDURE” / “BEGIN” / “VAR”";
		syntaxError(tempError);
		//ReadNextToken();
	}
	return t;
}

TreeNode* RecursiveDescentParsing::VarDeclaration(void)
{
	match(VAR);
	TreeNode* t = VarDecList();
	if (t == nullptr)
		syntaxError("变量声明缺失");
	return t;
}

TreeNode* RecursiveDescentParsing::VarDecList(void)
{
	TreeNode* t = new TreeNode;
	if (t == nullptr)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++)
			t->child[i] = nullptr;
		t->sibling = nullptr;
		t->nodekind = DecK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = nullptr;
		}
	}

	TreeNode* p = nullptr;
	if (t != nullptr)
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

//?< varDecMore > ::=  ε |  varDecList 
TreeNode* RecursiveDescentParsing::VarDecMore(void)
{
	TreeNode* t = nullptr;
	if (token.word.Lex == PROCEDURE || token.word.Lex == BEGIN){}
	else if (token.word.Lex == INTEGER || token.word.Lex == CHAR1 || token.word.Lex == ARRAY ||
			 token.word.Lex == RECORD || token.word.Lex == ID)
		t = VarDecList();
	else
	{
		string tempError = "意外符号\"" + token.word.Sem + "\"" + "，或许缺少符号“PROCEDURE” / “BEGIN” / “INTEGER” / “CHAR1” / “ARRAY” / “RECORD” / “变量名”";
		syntaxError(tempError);
		//ReadNextToken();
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
		syntaxError("变量名缺失");
		//ReadNextToken();
	}
	VarIdMore(t);
}

//?< varIdMore > ::=  ε |  , varIdList 
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
		string tempError = "意外符号\"" + token.word.Sem + "\"" + "，或许缺少符号“；” / “，”";
		syntaxError(tempError);
		//ReadNextToken();
	}
}

//?< procDec > ::=  ε |  procDeclaration
TreeNode* RecursiveDescentParsing::ProcDec(void)
{
	TreeNode* t = nullptr;
	if (token.word.Lex == BEGIN){}
	else if (token.word.Lex == PROCEDURE)
		t = ProcDeclaration();
	else
	{
		string tempError = "意外符号\"" + token.word.Sem + "\"" + "，或许缺少符号“PROCEDURE” / “BEGIN”";
		syntaxError(tempError);
		//ReadNextToken();
	}
	return t;
}

TreeNode* RecursiveDescentParsing::ProcDeclaration(void)
{
	TreeNode* t = new TreeNode;
	if (t == nullptr)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) t->child[i] = nullptr;
		t->sibling = nullptr;
		t->nodekind = ProcDecK;
		t->lineno = token.lineShow;
		t->idnum = 0;

		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = nullptr;
		}
	}

	match(PROCEDURE);
	if (t != nullptr)
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
	TreeNode* p = nullptr;
	if (token.word.Lex == RPAREN){}
	else if (token.word.Lex == INTEGER || token.word.Lex == CHAR1 || token.word.Lex == ARRAY ||
			 token.word.Lex == RECORD || token.word.Lex == ID || token.word.Lex == VAR)
	{
		p = ParamDecList();
		t->child[0] = p;
	}
	else
	{
		string tempError = "意外符号\"" + token.word.Sem + "\"";
		syntaxError(tempError);
		//ReadNextToken();
	}
}

TreeNode* RecursiveDescentParsing::ParamDecList(void)
{
	TreeNode * t = Param();
	TreeNode * p = ParamMore();
	if (p!=nullptr)
	{
		t->sibling = p;
	}
	return t;
}

//?< paramMore > ::=  ε | ; paramDecList 
TreeNode* RecursiveDescentParsing::ParamMore(void)
{
	TreeNode* t = nullptr;
	if (token.word.Lex == RPAREN){}
	else if (token.word.Lex == SEMI)
	{
		match(SEMI);
		t = ParamDecList();
		if (t == nullptr)
			syntaxError("参数缺失");
	}
	else
	{
		string tempError = "意外符号\"" + token.word.Sem + "\"" + "，或许缺少符号“；” / “）”";
		syntaxError(tempError);
		//ReadNextToken();
	}
	return t;
}

//?< param > ::=  typeName formList | VAR typeName formList
TreeNode* RecursiveDescentParsing::Param(void)
{
	TreeNode* t = new TreeNode;
	if (t == nullptr)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++)
			t->child[i] = nullptr;
		t->sibling = nullptr;
		t->nodekind = DecK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = nullptr;
		}
	}

	if (t != nullptr)
	{
		t->lineno = line0;
		if (token.word.Lex == INTEGER || token.word.Lex == CHAR1 || token.word.Lex == ARRAY ||
			token.word.Lex == RECORD || token.word.Lex == ID)
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
			string tempError = "意外符号\"" + token.word.Sem + "\"";
			syntaxError(tempError);
			//ReadNextToken();
		}
	}
	return t;
}

void RecursiveDescentParsing::FormList(TreeNode* t)
{
	if (t != nullptr)
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
		string tempError = "意外符号\"" + token.word.Sem + "\"" + "，或许缺少符号“；”/ “）” / “，”";
		syntaxError(tempError);
		//ReadNextToken();
	}
}

TreeNode* RecursiveDescentParsing::ProcDecPart(void)
{
	//newDecANode(typeK);
	TreeNode* typeP = new TreeNode;
	if (typeP == nullptr)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) 
			typeP->child[i] = nullptr;
		typeP->sibling = nullptr;
		typeP->nodekind = TypeK;
		typeP->lineno = token.lineShow;
		for (int i = 0; i < 10; i++)
		{
			typeP->name[i] = "\0";
			typeP->table[i] = nullptr;
		}
	}

	TreeNode* pp = typeP;
	if (typeP != nullptr)
	{
		typeP->lineno = 0;
		TreeNode* tp1 = TypeDec();
		if (tp1 != nullptr)
			typeP->child[0] = tp1;
		else
		{
			free(typeP);
			typeP = nullptr;
		}
	}

	//newDecANode(VarK);
	TreeNode* varP = new TreeNode;
	if (varP == nullptr)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++)
			varP->child[i] = nullptr;
		varP->sibling = nullptr;
		varP->nodekind = VarK;
		varP->lineno = token.lineShow;
		for (int i = 0; i < 10; i++)
		{
			varP->name[i] = "\0";
			varP->table[i] = nullptr;
		}
	}

	if (varP != nullptr)
	{
		varP->lineno = 0;
		TreeNode* tp2 = VarDec();
		if (tp2 != nullptr)
			varP->child[0] = tp2;
		else
		{
			free(varP);
			varP = nullptr;
		}
	}
	TreeNode* s = ProcDec();
	if (s == nullptr) {}

	if (varP == nullptr) 
		varP = s;

	if (typeP == nullptr) 
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
	if (t == nullptr)
		syntaxError("程序体缺失");
	return t;
}

TreeNode* RecursiveDescentParsing::ProgramBody(void)
{
	//newStmlNode();
	TreeNode* t = new TreeNode;
	if (t == nullptr)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) t->child[i] = nullptr;
		t->sibling = nullptr;
		t->nodekind = StmLK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = nullptr;
		}
	}

	match(BEGIN);
	if (t != nullptr)
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
	if (t != nullptr)
		if (p != nullptr)
			t->sibling = p;
	return t;
}

//?< stmMore > ::=   ε |  ; stmList 
TreeNode* RecursiveDescentParsing::StmMore(void)
{
	TreeNode* t = nullptr;
	if (token.word.Lex == FI || token.word.Lex == ELSE || token.word.Lex == END || token.word.Lex == ENDWH){}
	else if (token.word.Lex == SEMI)
	{
		match(SEMI);
		t = StmList();
	}
	else
	{
		string tempError = "意外符号\"" + token.word.Sem + "\"" + "，或许缺少符号“；”/ “END” / “ENDWH”";
		syntaxError(tempError);
		//ReadNextToken();
	}
	return t;
}

//? < stm > ::=   conditionalStm   {IF}      | loopStm          {WHILE}
//	            | inputStm         {READ}    | outputStm        {WRITE}
//	            | returnStm        {RETURN}  | id  assCall      {id}
TreeNode* RecursiveDescentParsing::Stm(void)
{
	TreeNode* t = nullptr;
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
		string tempError = "意外符号\"" + token.word.Sem + "\"";
		syntaxError(tempError);
		//ReadNextToken();
	}
	return t;
}

//?< assCall > ::=   assignmentRest  {:=,LMIDPAREN,DOT}  | callStmRest  {(} 
TreeNode* RecursiveDescentParsing::AssCall(void)
{
	TreeNode* t = nullptr;
	if (token.word.Lex == ASSIGN || token.word.Lex == DOT || token.word.Lex == LMIDPAREN)
		t = AssignmentRest();
	else if (token.word.Lex == LPAREN)
		t = CallStmRest();
	else
	{
		string tempError = "意外符号\"" + token.word.Sem + "\"" + "，或许缺少符号“:=” / “（”";
		syntaxError(tempError);
		//ReadNextToken();
	}
	return t;
}

TreeNode* RecursiveDescentParsing::AssignmentRest(void)
{
	//newStmtNode()
	TreeNode* t = new TreeNode;
	if (t == nullptr)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) t->child[i] = nullptr;
		t->sibling = nullptr;
		t->nodekind = StmtK;
		t->kind.stmt = AssignK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = nullptr;
		}
	}

	if (t != nullptr)
	{
		t->lineno = line0;

		//newExpNode
		TreeNode* child1 = new TreeNode;
		if (child1 == nullptr)
			syntaxError("内存泄露");
		else {
			for (int i = 0; i < 3; i++)
				child1->child[i] = nullptr;
			child1->sibling = nullptr;
			child1->nodekind = ExpK;
			child1->kind.exp = VariK;
			child1->lineno = token.lineShow;
			child1->attr.ExpAttr.varkind = IdV;
			child1->attr.ExpAttr.type = Void;
			for (int i = 0; i < 10; i++)
			{
				child1->name[i] = "\0";
				child1->table[i] = nullptr;
			}
		}

		if (child1 != nullptr)
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
	if (t == nullptr)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) t->child[i] = nullptr;
		t->sibling = nullptr;
		t->nodekind = StmtK;
		t->kind.stmt = IfK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = nullptr;
		}
	}

	match(IF);
	if (t != nullptr)
	{
		t->lineno = line0;
		t->child[0] = Exp();
	}
	match(THEN);
	if (t != nullptr)  t->child[1] = StmList();
	if (token.word.Lex == ELSE)
	{
		match(ELSE);
		if (t != nullptr)
			t->child[2] = StmList();
	}
	match(FI);
	return t;
}

TreeNode* RecursiveDescentParsing::LoopStm(void)
{
	//newStmtNode()
	TreeNode* t = new TreeNode;
	if (t == nullptr)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) t->child[i] = nullptr;
		t->sibling = nullptr;
		t->nodekind = StmtK;
		t->kind.stmt = WhileK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = nullptr;
		}
	}

	match(WHILE);
	if (t != nullptr)
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
	if (t == nullptr)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) t->child[i] = nullptr;
		t->sibling = nullptr;
		t->nodekind = StmtK;
		t->kind.stmt = ReadK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = nullptr;
		}
	}

	match(READ);
	match(LPAREN);
	if ((t != nullptr) && (token.word.Lex == ID))
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
	if (t == nullptr)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) t->child[i] = nullptr;
		t->sibling = nullptr;
		t->nodekind = StmtK;
		t->kind.stmt = WriteK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = nullptr;
		}
	}

	match(WRITE);
	match(LPAREN);
	if (t != nullptr)
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
	if (t == nullptr)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) t->child[i] = nullptr;
		t->sibling = nullptr;
		t->nodekind = StmtK;
		t->kind.stmt = ReturnK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = nullptr;
		}
	}

	match(RETURN);
	if (t != nullptr)
		t->lineno = line0;
	return t;
}

TreeNode* RecursiveDescentParsing::CallStmRest(void)
{
	//newStmtNode()
	TreeNode* t = new TreeNode;
	if (t == nullptr)
		syntaxError("内存溢出");
	else {
		for (int i = 0; i < 3; i++) t->child[i] = nullptr;
		t->sibling = nullptr;
		t->nodekind = StmtK;
		t->kind.stmt = CallK;
		t->lineno = token.lineShow;
		t->idnum = 0;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = nullptr;
		}
	}

	match(LPAREN);
	/*函数调用时，其子节点指向实参*/
	if (t != nullptr)
	{
		t->lineno = line0;

		/*函数名的结点也用表达式类型结点*/
		TreeNode* child0 = new TreeNode;
		if (child0 == nullptr)
			syntaxError("内存溢出");
		else {
			for (int i = 0; i < 3; i++) 
				child0->child[i] = nullptr;
			child0->sibling = nullptr;
			child0->nodekind = ExpK;
			child0->kind.exp = VariK;
			child0->lineno = token.lineShow;
			child0->attr.ExpAttr.varkind = IdV;
			child0->attr.ExpAttr.type = Void;
			for (int i = 0; i < 10; i++)
			{
				child0->name[i] = "\0";
				child0->table[i] = nullptr;
			}
		}

		if (child0 != nullptr)
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
	TreeNode* t = nullptr;
	if (token.word.Lex == RPAREN) {}
	else if (token.word.Lex == ID || token.word.Lex == INTC || token.word.Lex == LPAREN)
	{
		t = Exp();
		if (t != nullptr)
			t->sibling = ActParamMore();
	}
	else
	{
		string tempError = "意外符号\"" + token.word.Sem + "\"" + "，或许缺少符号“变量名” / “）” / “整形”";
		syntaxError(tempError);
		//ReadNextToken();
	}
	return t;
}

//?< actParamMore > ::=     ε |  , actParamList 
TreeNode* RecursiveDescentParsing::ActParamMore(void)
{
	TreeNode* t = nullptr;
	if (token.word.Lex == RPAREN){}
	else if (token.word.Lex == COMMA)
	{
		match(COMMA);
		t = ActParamList();
	}
	else
	{
		string tempError = "意外符号\"" + token.word.Sem + "\"" + "，或许缺少符号“）”/ “，”";
		syntaxError(tempError);
		//ReadNextToken();
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
		if (p == nullptr)
			syntaxError("内存泄露");
		else {
			for (int i = 0; i < 3; i++)
				p->child[i] = nullptr;
			p->sibling = nullptr;
			p->nodekind = ExpK;
			p->kind.exp = OpK;
			p->lineno = token.lineShow;
			p->attr.ExpAttr.varkind = IdV;
			p->attr.ExpAttr.type = Void;
			for (int i = 0; i < 10; i++)
			{
				p->name[i] = "\0";
				p->table[i] = nullptr;
			}
		}

		if (p != nullptr)
		{
			p->lineno = line0;
			p->child[0] = t;
			p->attr.ExpAttr.op = token.word.Lex;
			t = p;
		}
		match(token.word.Lex);
		if (t != nullptr)
			t->child[1] = Simple_exp();
	}
	return t;
}

TreeNode* RecursiveDescentParsing::Simple_exp(void)
{
	TreeNode* t = Term();
	while ((token.word.Lex == PLUS) || (token.word.Lex == MINUS))
	{
		//newExpNode()
		TreeNode* p = new TreeNode;
		if (p == nullptr)
			syntaxError("内存泄露");
		else {
			for (int i = 0; i < 3; i++)
				p->child[i] = nullptr;
			p->sibling = nullptr;
			p->nodekind = ExpK;
			p->kind.exp = OpK;
			p->lineno = token.lineShow;
			p->attr.ExpAttr.varkind = IdV;
			p->attr.ExpAttr.type = Void;
			for (int i = 0; i < 10; i++)
			{
				p->name[i] = "\0";
				p->table[i] = nullptr;
			}
		}

		if (p != nullptr)
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
		if (p == nullptr)
			syntaxError("内存泄露");
		else {
			for (int i = 0; i < 3; i++)
				p->child[i] = nullptr;
			p->sibling = nullptr;
			p->nodekind = ExpK;
			p->kind.exp = OpK;
			p->lineno = token.lineShow;
			p->attr.ExpAttr.varkind = IdV;
			p->attr.ExpAttr.type = Void;
			for (int i = 0; i < 10; i++)
			{
				p->name[i] = "\0";
				p->table[i] = nullptr;
			}
		}

		if (p != nullptr)
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
	TreeNode* t = nullptr;
	if (token.word.Lex == INTC)
	{
		//newExpNode()
		t = new TreeNode;
		if (t == nullptr)
			syntaxError("内存泄露");
		else {
			for (int i = 0; i < 3; i++)
				t->child[i] = nullptr;
			t->sibling = nullptr;
			t->nodekind = ExpK;
			t->kind.exp = ConstK;
			t->lineno = token.lineShow;
			t->attr.ExpAttr.varkind = IdV;
			t->attr.ExpAttr.type = Void;
			for (int i = 0; i < 10; i++)
			{
				t->name[i] = "\0";
				t->table[i] = nullptr;
			}
		}

		if ((t != nullptr) && (token.word.Lex == INTC))
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
		string tempError = "意外符号\"" + token.word.Sem + "\"" + "，或许缺少符号“变量名” /“（” / “整形”";
		syntaxError(tempError);
		//ReadNextToken();
	}
	return t;
}

TreeNode* RecursiveDescentParsing::Variable(void)
{
	//newExpNode()
	TreeNode* t = new TreeNode;
	if (t == nullptr)
		syntaxError("内存泄露");
	else {
		for (int i = 0; i < 3; i++)
			t->child[i] = nullptr;
		t->sibling = nullptr;
		t->nodekind = ExpK;
		t->kind.exp = VariK;
		t->lineno = token.lineShow;
		t->attr.ExpAttr.varkind = IdV;
		t->attr.ExpAttr.type = Void;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = nullptr;
		}
	}

	if ((t != nullptr) && (token.word.Lex == ID))
	{
		t->lineno = line0;
		t->name[0] = token.word.Sem;
		(t->idnum)++;
	}
	match(ID);
	VariMore(t);
	return t;
}

//?variMore   ::=  ε                             			
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
		string tempError = "意外符号\"" + token.word.Sem + "\"";
		syntaxError(tempError);
		//ReadNextToken();
	}
}

TreeNode* RecursiveDescentParsing::Fieldvar(void)
{
	//newExpNode
	TreeNode* t = new TreeNode;
	if (t == nullptr)
		syntaxError("内存泄露");
	else {
		for (int i = 0; i < 3; i++)
			t->child[i] = nullptr;
		t->sibling = nullptr;
		t->nodekind = ExpK;
		t->kind.exp = VariK;
		t->lineno = token.lineShow;
		t->attr.ExpAttr.varkind = IdV;
		t->attr.ExpAttr.type = Void;
		for (int i = 0; i < 10; i++)
		{
			t->name[i] = "\0";
			t->table[i] = nullptr;
		}
	}

	if ((t != nullptr) && (token.word.Lex == ID))
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
		string tempError = "意外符号\"" + token.word.Sem + "\"";
		syntaxError(tempError);
		//ReadNextToken();
	}
}

void RecursiveDescentParsing::printTree(TreeNode* tree)
{
	//fopen_s(&listing, "treeFile.txt", "w");
	//if (Error == false)
	//{
		indentation += 4;		//缩进加4

		while (tree != nullptr)
		{
			//打印行号
			if (tree->lineno == 0)
				for (int i = 0; i < 9; i++)
					treeFile << ' ';
			else
				//每10行一标记
				switch (tree->lineno / 10)
				{
					case 0:
						treeFile << "line: " << tree->lineno;
						for (int i = 0; i < 3; i++)
							treeFile << " ";
						break;
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:
						treeFile << "line: " << tree->lineno;
						for (int i = 0; i < 2; i++)
							treeFile << " ";
						break;
					default:
						treeFile << "line: " << tree->lineno;
						for (int i = 0; i < 1; i++)
							treeFile << " ";
				}

			//缩进
			for (int i = 0; i < indentation; i++)
				treeFile << " ";

			//判断nodeKind
			switch (tree->nodekind)
			{
				case ProK:
				{
					treeFile << "ProK  "; 
					setbkcolor(RGB(255, 255, 255));
					setcolor(RGB(0,0,0));
					settextcolor(RGB(0, 0, 0));

					_stprintf_s(temp, _T("Prok"));
					circleX = 650, circleY = 50, circleR = 40;
					circle(circleX, circleY, circleR); // 画圆
					outtextxy(circleX - 20, circleY - 10, temp);// 文字
				}
					break;
					
				case PheadK:
				{
					treeFile << "PheadK  ";

					_stprintf_s(temp, _T("PheadK"));
					circleX = 50, circleY = 150, circleR = 40;
					circle(circleX, circleY, circleR); // 画圆
					outtextxy(circleX - 20, circleY - 10, temp);// 文字
					line(circleX, circleY - 40, 610 , 50);

					treeFile << tree->name[0] << "  ";
				}
					break;

				case DecK:
				{  
					treeFile << "Deck  ";
					if (tree->attr.ProcAttr.paramt == varparamType)
						treeFile << "var param:  ";
					if (tree->attr.ProcAttr.paramt == valparamType)
						treeFile << "value param:  ";

					switch (tree->kind.dec)
					{
						case  ArrayK:
						{
							treeFile << "ArrayK  ";
							treeFile << tree->attr.ArrayAttr.up << "  ";
							treeFile << tree->attr.ArrayAttr.low << "  ";
							if (tree->attr.ArrayAttr.childtype == CharK)
								treeFile << "Chark  ";
							else if (tree->attr.ArrayAttr.childtype == IntegerK)
								treeFile << "Integer  ";

							if (lastNode == VarK && lastIsDeck == false)
							{
								_stprintf_s(temp, _T("Dec: ArrK"));
								circleX = 250, circleY = 250, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 30, circleY - 10, temp);// 文字
								line(circleX, circleY - 40, 450, 190);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
							else if (lastNode == VarK && lastIsDeck == true)
							{
								_stprintf_s(temp, _T("Dec: ArrK"));
								circleX = lastX + 60, circleY = lastY, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 30, circleY - 10, temp);// 文字
								line(circleX - 40, circleY, lastX, lastY);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
							else if (lastNode == TypeK && lastIsDeck == false)
							{
								_stprintf_s(temp, _T("Dec: ArrK"));
								circleX = 50, circleY = 350, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 30, circleY - 10, temp);// 文字
								line(circleX, circleY - 40, 250, 190);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
							else if (lastNode == TypeK && lastIsDeck == true)
							{
								_stprintf_s(temp, _T("Dec: ArrK"));
								circleX = lastX + 60, circleY = lastY, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 30, circleY - 10, temp);// 文字
								line(circleX - 40, circleY, lastX, lastY);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
						}; 
						break;

						case  CharK:
						{
							treeFile << "Chark  ";

							if (lastNode == VarK && lastIsDeck == false)
							{
								_stprintf_s(temp, _T("Dec: CharK"));
								circleX = 250, circleY = 250, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 34, circleY - 10, temp);// 文字
								line(circleX, circleY - 40, 450, 190);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
							else if (lastNode == VarK && lastIsDeck == true)
							{
								_stprintf_s(temp, _T("Dec: CharK"));
								circleX = lastX + 60, circleY = lastY, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 34, circleY - 10, temp);// 文字
								line(circleX - 40, circleY, lastX, lastY);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
							else if (lastNode == TypeK && lastIsDeck == false)
							{
								_stprintf_s(temp, _T("Dec: CharK"));
								circleX = 50, circleY = 350, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 34, circleY - 10, temp);// 文字
								line(circleX, circleY - 40, 250, 190);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
							else if (lastNode == TypeK && lastIsDeck == true)
							{
								_stprintf_s(temp, _T("Dec: CharK"));
								circleX = lastX + 60, circleY = lastY, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 34, circleY - 10, temp);// 文字
								line(circleX - 40, circleY, lastX, lastY);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
						}
							break;
						case  IntegerK:
						{
							treeFile << "Integer  ";

							if (lastNode == VarK && lastIsDeck == false)
							{
								_stprintf_s(temp, _T("Dec: IntK"));
								circleX = 250, circleY = 250, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 30, circleY - 10, temp);// 文字
								line(circleX, circleY - 40, 450, 190);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
							else if (lastNode == VarK && lastIsDeck == true)
							{
								_stprintf_s(temp, _T("Dec: IntK"));
								circleX = lastX + 60, circleY = lastY, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 30, circleY - 10, temp);// 文字
								line(circleX - 40, circleY, lastX, lastY);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
							else if (lastNode == TypeK && lastIsDeck == false)
							{
								_stprintf_s(temp, _T("Dec: IntK"));
								circleX = 50, circleY = 350, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 30, circleY - 10, temp);// 文字
								line(circleX, circleY - 40, 250, 190);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
							else if (lastNode == TypeK && lastIsDeck == true)
							{
								_stprintf_s(temp, _T("Dec: IntK"));
								circleX = lastX + 60, circleY = lastY, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 30, circleY - 10, temp);// 文字
								line(circleX - 40, circleY, lastX, lastY);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
						}
							break;

						case  RecordK:
						{
							treeFile << "RecordK  ";

							if (lastNode == VarK && lastIsDeck == false)
							{
								_stprintf_s(temp, _T("Dec: ReK"));
								circleX = 250, circleY = 250, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 30, circleY - 10, temp);// 文字
								line(circleX, circleY - 40, 450, 190);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
							else if (lastNode == VarK && lastIsDeck == true)
							{
								_stprintf_s(temp, _T("Dec: ReK"));
								circleX = lastX + 60, circleY = lastY, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 30, circleY - 10, temp);// 文字
								line(circleX - 40, circleY, lastX, lastY);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
							else if (lastNode == TypeK && lastIsDeck == false)
							{
								_stprintf_s(temp, _T("Dec: ReK"));
								circleX = 50, circleY = 350, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 30, circleY - 10, temp);// 文字
								line(circleX, circleY - 40, 250, 190);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
							else if (lastNode == TypeK && lastIsDeck == true)
							{
								_stprintf_s(temp, _T("Dec: ReK"));
								circleX = lastX + 60, circleY = lastY, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 30, circleY - 10, temp);// 文字
								line(circleX - 40, circleY, lastX, lastY);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
						}
							break;
						case  IdK:
						{
							treeFile << "IdK  ";
							treeFile << tree->attr.type_name << "  ";

							if (lastNode == VarK && lastIsDeck == false)
							{
								_stprintf_s(temp, _T("Dec: IdK"));
								circleX = 250, circleY = 250, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 30, circleY - 10, temp);// 文字
								line(circleX, circleY - 40, 450, 190);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
							else if (lastNode == VarK && lastIsDeck == true)
							{
								_stprintf_s(temp, _T("Dec: IdK"));
								circleX = lastX + 60, circleY = lastY, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 30, circleY - 10, temp);// 文字
								line(circleX - 40, circleY, lastX, lastY);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
							else if (lastNode == TypeK && lastIsDeck == false)
							{
								_stprintf_s(temp, _T("Dec: IdK"));
								circleX = 50, circleY = 350, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 30, circleY - 10, temp);// 文字
								line(circleX, circleY - 40, 250, 190);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
							else if (lastNode == TypeK && lastIsDeck == true)
							{
								_stprintf_s(temp, _T("Dec: IdK"));
								circleX = lastX + 60, circleY = lastY, circleR = 40;
								circle(circleX, circleY, circleR); // 画圆
								outtextxy(circleX - 30, circleY - 10, temp);// 文字
								line(circleX - 40, circleY, lastX, lastY);
								lastX = circleX + 40;
								lastY = circleY;
								lastIsDeck = true;
							}
						}
							break;
						default:
							treeFile << "error1!  ";
							Error = true;
					};
					if (tree->idnum != 0)
						for (int i = 0; i <= (tree->idnum); i++)
							treeFile << tree->name[i] << "  ";
					else
					{
						treeFile << "wrong!no var!\n";
						Error = true;
					}
				} 
					break;

				case TypeK:
				{
					treeFile << "TypeK  ";

					_stprintf_s(temp, _T("TypeK"));
					circleX = 250, circleY = 150, circleR = 40;
					circle(circleX, circleY, circleR); // 画圆
					outtextxy(circleX - 20, circleY - 10, temp);// 文字
					line(circleX, circleY - 40, 610, 50);
					lastNode = tree->nodekind;
					lastIsDeck = false;
					hasTypeK = true;
				}
					break;

				case VarK:
				{
					treeFile << "VarK  ";
					//if (tree->table[0] != nullptr)
					//	fprintf(listing, "%d  %d  ", tree->table[0]->attrIR.More.VarAttr.off, tree->table[0]->attrIR.More.VarAttr.level);
					
					_stprintf_s(temp, _T("VarK"));
					circleX = 450, circleY = 150, circleR = 40;
					circle(circleX, circleY, circleR); // 画圆
					outtextxy(circleX - 20, circleY - 10, temp);// 文字
					if(hasTypeK == true)
						line(circleX - 40, circleY, 290, 150);
					else
						line(circleX, circleY - 40, 610, 50);
					lastNode = tree->nodekind;
					lastIsDeck = false;
				}					
					break;

				case ProcDecK:
				{
					treeFile << "ProcDecK  ";
					treeFile << tree->name[0] << "  ";
					//if (tree->table[0] != nullptr)
					//	fprintf(listing, "%d %d %d  ", tree->table[0]->attrIR.More.ProcAttr.mOff, tree->table[0]->attrIR.More.ProcAttr.nOff, tree->table[0]->attrIR.More.ProcAttr.level);
					
					stmlkNum++;
					//代码中有声明，则添加PORC节点
					if (procNum == 0)
					{
						_stprintf_s(temp, _T("ProcK"));
						circleX = 650, circleY = 150, circleR = 40;
						circle(circleX, circleY, circleR); // 画圆
						outtextxy(circleX - 25, circleY - 10, temp);// 文字
						line(circleX - 40, circleY, 490, 150);
						lastNode = tree->nodekind;
						lastIsDeck = false;
						procNum++;

						_stprintf_s(temp, _T("Dec: Proc"));
						circleX = 250, circleY = 550, circleR = 40;
						circle(circleX, circleY, circleR); // 画圆
						outtextxy(circleX - 30, circleY - 10, temp);// 文字
						line(circleX, circleY - 40, 650, 190);
						lastX = circleX + 40;
						lastY = circleY;
						lastIsDeck = true;
					}
					else
					{
						_stprintf_s(temp, _T("Dec: Proc"));
						circleX = lastX + 60, circleY = lastY, circleR = 40;
						circle(circleX, circleY, circleR); // 画圆
						outtextxy(circleX - 30, circleY - 10, temp);// 文字
						line(circleX - 40, circleY, lastX, lastY);
						lastX = circleX + 40;
						lastY = circleY;
						lastIsDeck = true;
					}

				}					
					break;

				case StmLK:
				{
					treeFile << "StmLk  ";
					if (stmlkNum != 1)
						stmlkNum--;
					if (stmlkNum == 1)
					{
						_stprintf_s(temp, _T("StmLk"));
						circleX = 850, circleY = 150, circleR = 40;
						circle(circleX, circleY, circleR); // 画圆
						outtextxy(circleX - 20, circleY - 10, temp);// 文字
						line(circleX, circleY - 40, 690, 50);
						lastNode = tree->nodekind;
						lastIsDeck = false;
					}
				}
					break;

				case StmtK:
				{ 
					treeFile << "Stmtk  ";

					if (stmlkNum == 1)
					{
						if (lastIsDeck == false)
						{
							_stprintf_s(temp, _T("Stmtk:"));
							circleX = 450, circleY = 450, circleR = 40;
							circle(circleX, circleY, circleR); // 画圆
							outtextxy(circleX - 20, circleY - 15, temp);// 文字
							line(circleX, circleY - 40, 850, 190);
							lastX = circleX + 40;
							lastY = circleY;
							lastIsDeck = true;
						}
						else if (lastIsDeck == true)
						{
							_stprintf_s(temp, _T("Stmtk:"));
							circleX = lastX + 60, circleY = lastY, circleR = 40;
							circle(circleX, circleY, circleR); // 画圆
							outtextxy(circleX - 20, circleY - 15, temp);// 文字
							line(circleX - 40, circleY, lastX, lastY);
							lastX = circleX + 40;
							lastY = circleY;
							lastIsDeck = true;
						}
					}

					switch (tree->kind.stmt)
					{
						case IfK:
							treeFile << "If  ";
							_stprintf_s(temp, _T("If"));
							if (stmlkNum == 1)
								outtextxy(circleX - 15, circleY + 7, temp);
							break;

						case WhileK:
							treeFile << "While  ";
							_stprintf_s(temp, _T("While"));
							if (stmlkNum == 1)
								outtextxy(circleX - 15, circleY + 7, temp);
							break;

						case AssignK:
							treeFile << "Assign  "; 
							_stprintf_s(temp, _T("Assign"));
							if (stmlkNum == 1)
								outtextxy(circleX - 15, circleY + 7, temp);
							break;

						case ReadK:
							treeFile << "Read  ";
							treeFile << tree->name[0] << "  ";
							_stprintf_s(temp, _T("Read"));
							if (stmlkNum == 1)
								outtextxy(circleX - 15, circleY + 7, temp);
							//if (tree->table[0] != nullptr)
							//	fprintf(listing, "%d   %d  ", tree->table[0]->attrIR.More.VarAttr.off, tree->table[0]->attrIR.More.VarAttr.level);
							break;

						case WriteK:
							treeFile << "Write  ";
							_stprintf_s(temp, _T("Write"));
							if (stmlkNum == 1)
								outtextxy(circleX - 15, circleY + 7, temp);
							break;

						case CallK:
							treeFile << "Call  ";
							treeFile << tree->name[0] << "  ";
							_stprintf_s(temp, _T("Call"));
							if (stmlkNum == 1)
								outtextxy(circleX - 15, circleY + 7, temp);
							break;

						case ReturnK:
							treeFile << "Return  "; 
							_stprintf_s(temp, _T("Return"));
							if (stmlkNum == 1)
								outtextxy(circleX - 15, circleY + 7, temp);
							break;

						default:
							treeFile << "error2!";
							Error = true;
					}
				}; 
					break;

				case ExpK:
				{ 
					treeFile << "ExpK  ";
					switch (tree->kind.exp)
					{
						case OpK:
						{ 
							treeFile << "Op  ";
							switch (tree->attr.ExpAttr.op)
							{
								case EQ:   treeFile << "=  "; break;
								case LT:   treeFile << "<  "; break;
								case PLUS: treeFile << "+  "; break;
								case MINUS:treeFile << "-  "; break;
								case TIMES:treeFile << "*  "; break;
								case OVER: treeFile << "/  "; break;
								default:
									treeFile << "error2!";
									Error = true;
							}

							if (tree->attr.ExpAttr.varkind == ArrayMembV)
							{
								treeFile << "ArrayMember  ";
								treeFile << tree->name[0] << "  ";
							}
						}; 
							break;

						case ConstK:
							treeFile << "Const  ";
							switch (tree->attr.ExpAttr.varkind)
							{
								case IdV:
									treeFile << "Id  ";
									treeFile << tree->name[0] << "  ";
									break;
								case FieldMembV:
									treeFile << "FieldMember  ";
									treeFile << tree->name[0] << "  ";
									break;
								case ArrayMembV:
									treeFile << "ArrayMember  ";
									treeFile << tree->name[0] << "  ";
									break;
								default:
									treeFile << "var type error!";
									Error = true;
							}
							treeFile << tree->attr.ExpAttr.val << "  ";
							break;

						case VariK:
							treeFile << "Vari  ";
							switch (tree->attr.ExpAttr.varkind)
							{
								case IdV:
									treeFile << "Id  ";
									treeFile << tree->name[0] << "  ";
									break;
								case FieldMembV:
									treeFile << "FieldMember  ";
									treeFile << tree->name[0] << "  ";
									break;
								case ArrayMembV:
									treeFile << "ArrayMember  ";
									treeFile << tree->name[0] << "  ";
									break;
								default:
									treeFile << "var type error!";
									Error = true;
							}
							//if (tree->table[0] != nullptr)
							//	fprintf(listing, "%d   %d  ", tree->table[0]->attrIR.More.VarAttr.off, tree->table[0]->attrIR.More.VarAttr.level);
							break;

						default:
							treeFile << "error4!";
							Error = true;
					}
				}; 
					break;

				default:
					treeFile << "error5!";
					Error = true;
			}

			treeFile << "\n";

			for (int i = 0; i < 3; i++)
				printTree(tree->child[i]);

			tree = tree->sibling;
		}
		indentation -= 4;		//缩进减4
	//}
	//else
	//	treeFile << "存在语法错误，语法树生成失败!";
	//fclose(listing);
}