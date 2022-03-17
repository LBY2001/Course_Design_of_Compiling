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

//���ʵ�����ö�٣��ο�����P32ҳ
typedef enum
{
	/* ���ǵ��ʷ��� */
	ENDFILE1, ERROR1,

	/* ������ */
	PROGRAM, PROCEDURE, TYPE, VAR, IF,
	THEN, ELSE, FI, WHILE, DO, ENDWH,
	BEGIN, END1, READ, WRITE, ARRAY, OF,
	RECORD, RETURN1,

	INTEGER, CHAR1,

	/* ���ַ����ʷ��� */
	ID, INTC, CHARC,

	/*������� */
	ASSIGN, EQ, LT, PLUS, MINUS,
	TIMES, OVER, LPAREN, RPAREN, DOT,
	COLON, SEMI, COMMA, LMIDPAREN, RMIDPAREN,
	UNDERANGE
}LexType;

//���ʵ�������Ϣ
struct Word
{
	string Sem;		//������Ϣ
	LexType Lex;	//�ʷ���Ϣ
	Word(string sem, LexType lex)
	{
		Sem = sem;
		Lex = lex;
	}
	Word() {}
};

//SNL��Token�ṹ
struct Token 
{
	int lineShow;		//�õ�����Դ�����е�����
	struct Word word;
	Token(int lineShow, struct Word w)
	{
		this->lineShow = lineShow;
		word = w;
	}
	Token() {}
};

//�ʷ�����
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
		vector<Token*> TokenList;	//TokenList�Ľṹ
};