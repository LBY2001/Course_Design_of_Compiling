#pragma once

#include <iostream>
#include<fstream>
#include<string>
#include<unordered_map>
#include<algorithm>
#include<set>
#include<vector>
#include <iomanip>
#include <sstream>
using namespace std;

//单词的类型枚举，参考资料P32页
typedef enum
{
	/* 簿记单词符号 */
	ENDFILE1, ERROR1,

	/* 保留字 */
	PROGRAM, PROCEDURE, TYPE, VAR, IF,
	THEN, ELSE, FI, WHILE, DO, ENDWH,
	BEGIN, END1, READ, WRITE, ARRAY, OF,
	RECORD, RETURN1,

	INTEGER, CHAR1,

	/* 多字符单词符号 */
	ID, INTC, CHARC,

	/*特殊符号 */
	ASSIGN, EQ, LT, PLUS, MINUS,
	TIMES, OVER, LPAREN, RPAREN, DOT,
	COLON, SEMI, COMMA, LMIDPAREN, RMIDPAREN,
	UNDERANGE
}LexType;

//类型哈希表
unordered_map<int, string> ha =
{
	{0, "ENDFILE1"},	{1, "ERROR1"},		{2, "PROGRAM"},		{3,"PROCEDURE"},
	{4, "TYPE"},		{5, "VAR"},			{6, "IF"},			{7, "THEN"},
	{8, "ELSE"},		{9, "FI"},			{10, "WHILE"},		{11, "DO"},
	{12, "ENDWH"},		{13, "BEGIN"},		{14, "END1"},		{15, "READ"},
	{16, "WRITE"},		{11, "ARRAY"},		{12, "OF"},			{13, "RECORD"},
	{20, "RETURN1"},	{21, "INTEGER"},	{22, "CHAR1"},		{23, "ID"},
	{24, "INTC"},		{25, "CHARC"},		{26, "ASSIGN"},		{27, "EQ"},
	{28, "LT"},			{29, "PLUS"},		{30, "MINUS"},		{31, "TIMES"},
	{32, "OVER"},		{33, "LPAREN"},		{34, "RPAREN"},		{35, "DOT"},
	{36, "COLON"},		{37, "SEMI"},		{38, "COMMA"},		{39, "LMIDPAREN"},
	{40, "RMIDPAREN"},	{41, "UNDERANGE"}
};

//单词的语义信息
static struct Word
{
	string Sem;		//语义信息
	LexType Lex;	//词法信息
	Word(string sem, LexType lex)
	{
		Sem = sem;
		Lex = lex;
	}
	Word() {}
}keyWords[21] =
{
	{"program",PROGRAM},{"type",TYPE},{"var",VAR},
	{"procedure",PROCEDURE},{"begin",BEGIN},{"end",END1},{"array",ARRAY},
	{"of",OF},{"record",RECORD},{"if",IF},{"then",THEN},{"else",ELSE},{"fi",FI},
	{"while",WHILE},{"do",DO},{"endwh",ENDWH},{"read",READ},{"write",WRITE},
	{"return",RETURN1},{"integer",INTEGER},{"char",CHAR1}
};//保留字

//SNL的Token结构
struct Token 
{
	int lineShow;		//该单词在源程序中的行数
	struct Word word;
	Token(int lineShow, struct Word w)
	{
		this->lineShow = lineShow;
		word = w;
	}
	Token() {}
};

//词法分析
class LexicalAnalyzer
{
	public:
		LexicalAnalyzer();
		~LexicalAnalyzer();
		bool isLetter(char c);
		bool isDigit(char c);
		bool isDelimiter(char c);
		bool isOperator(char c);
		bool isFilter(char c);
		bool isKeyWord(string s);
		void getTokenList();
		vector<Token*> TokenList;	//TokenList的结构
};