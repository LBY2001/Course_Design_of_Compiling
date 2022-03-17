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

//单词的语义信息
struct Word
{
	string Sem;		//语义信息
	LexType Lex;	//词法信息
	Word(string sem, LexType lex)
	{
		Sem = sem;
		Lex = lex;
	}
	Word() {}
};

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