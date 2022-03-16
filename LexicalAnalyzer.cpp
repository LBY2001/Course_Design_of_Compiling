#include <iostream>
#include<fstream>
#include<string>
#include<unordered_map>
#include<algorithm>
#include<set>
#include<vector>
#include <iomanip>
#include <sstream>
#include"LexicalAnalyzer.h"
using namespace std;

LexicalAnalyzer::LexicalAnalyzer(){}

LexicalAnalyzer::~LexicalAnalyzer(){}

/*=====字符 / 字符串的类型判断=====*/
bool LexicalAnalyzer::isLetter(char c)
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

bool LexicalAnalyzer::isDigit(char c)
{
	return (c >= '0' && c <= '9');
}

bool LexicalAnalyzer::isDelimiter(char c)
{
	return (c == ';' || c == ',' || c == '{' || c == '}' || 
			c == '[' || c == ']' || c == '(' || c == ')' || 
			c == '.' || c == ':' || c == '\'');
}

bool LexicalAnalyzer::isOperator(char c)
{
	return (c == '+' || c == '-' || c == '*' || 
			c == '/' || c == '<' || c == '=');
}

bool LexicalAnalyzer::isFilter(char c)
{
	return (c == ' ' || c == '\t' || c == '\r' || c == '\n');
}

bool LexicalAnalyzer::isKeyWord(string s)
{
	for (int i = 0; i < 21; i++)
		if (s == keyWords[i].Sem)
			return true;
	return false;
}
/*==================================*/

void LexicalAnalyzer::getTokenList()
{
	//文件读入
	ifstream file;
	string inputFile;
	inputFile = "text.cpp";		//输入文件名
	file.open(inputFile);
	if (!file)
		cout << "文件打开失败！";

	FILE* f = nullptr;
	fopen_s(&f, inputFile.c_str(), "r");
	//文件内容分析
	int lineShow = 1;//当前代码行数
	char c = fgetc(f);
	while (c != EOF)
	{
		//判断前导白色空格字符
		if (isFilter(c))
		{
			if (c == '\n')
				lineShow++;
			c = fgetc(f);
		}

		//判断标识符 / 关键字
		else if (isLetter(c))
		{
			string currentString = "";
			currentString += c;
			c = fgetc(f);
			while (isLetter(c) || isDigit(c))
			{
				currentString += c;
				c = fgetc(f);
			}
			//判断关键字
			if (isKeyWord(currentString))
			{
				Word tempWord;
				Token* tempToken;
				for (int i = 0; i < 21; i++)
				{
					if (keyWords[i].Sem == currentString)
					{
						tempWord.Sem = keyWords[i].Sem;
						tempWord.Lex = keyWords[i].Lex;
					}
				}
				tempToken = new Token(lineShow, tempWord);
				TokenList.push_back(tempToken);
			}

			//判断标识符
			else
			{
				Word tempWord;
				Token* tempToken = new Token(lineShow, Word(currentString, ID));
				TokenList.push_back(tempToken);
			}
		}

		//判断数字
		else if (isDigit(c))
		{
			string currentString = "";
			currentString += c;
			c = fgetc(f);
			while (isDigit(c) || isLetter(c))
			{
				currentString += c;
				c = fgetc(f);
			}
			bool isERROR1 = false;			//判断字符串是否都由数字组成,否则为错误单词
			for (int i = 0; i < currentString.length(); i++)
			{
				if (isLetter(currentString[i]))
					isERROR1 = true;
			}

			//数字夹杂字母
			if (isERROR1) {
				Token* tempToken = new Token(lineShow, Word(currentString, ERROR1));
				TokenList.push_back(tempToken);
			}

			//纯数字，返回整形
			else
			{
				Token* tempToken = new Token(lineShow, Word(currentString, INTC));
				TokenList.push_back(tempToken);
			}
		}

		//判断运算符
		else if (isOperator(c))
		{
			LexType tokenType;
			if (c == '+') {
				tokenType = PLUS;
			}
			else if (c == '-') {
				tokenType = MINUS;
			}
			else if (c == '*') {
				tokenType = TIMES;
			}
			else if (c == '/' ) {
				tokenType = OVER;
			}
			else if (c == '<') {
				tokenType = LT;
			}
			else if (c == '=') {
				tokenType = EQ;
			}

			string temp(1, c);
			Token* tempToken = new Token(lineShow, Word(temp, tokenType));
			TokenList.push_back(tempToken);
			c = fgetc(f);
		}

		//判断分隔符
		else if (isDelimiter(c))
		{
			string currentString = "";
			Token* tempToken;
			//注释{}
			if (c == '{')
			{
				while (c != '}')
				{
					c = fgetc(f);
					if (c == '\n')
						lineShow += 1;
				}
				c = fgetc(f);
			}

			//判断数组下标限界符
			else if (c == '.')
			{
				currentString += c;
				if ((c = fgetc(f)) == '.')
				{
					currentString += c;
					tempToken = new Token(lineShow, Word(currentString, UNDERANGE));
					TokenList.push_back(tempToken);
					c = fgetc(f);
				}
				else
				{
					tempToken = new Token(lineShow, Word(currentString, ERROR1));
					TokenList.push_back(tempToken);
				}
			}

			//判断字符串
			else if (c == '\'')
			{
				while ((c = fgetc(f)) != '\'')
				{
					currentString += c;
				}
				tempToken = new Token(lineShow, Word(currentString, CHARC));
				TokenList.push_back(tempToken);
				c = fgetc(f);
			}

			//判断双字符分解符
			else if (c == ':')
			{
				currentString += c;
				if ((c = fgetc(f)) == '=')
				{
					currentString += c;
					tempToken = new Token(lineShow, Word(currentString, ASSIGN));
					TokenList.push_back(tempToken);
					c = fgetc(f);
				}
				else
				{
					tempToken = new Token(lineShow, Word(currentString, ERROR1));
					TokenList.push_back(tempToken);
				}
			}

			//判断其他符号
			else
			{
				LexType tokenType;
				string temp(1, c);
				if (temp == "(") {
					tokenType = LPAREN;
				}
				else if (temp == ")") {
					tokenType = RPAREN;
				}
				else if (temp == ".") {
					tokenType = DOT;
				}
				else if (temp == "[") {
					tokenType = LMIDPAREN;
				}
				else if (temp == "]") {
					tokenType = RMIDPAREN;
				}
				else if (temp == ";") {
					tokenType = SEMI;
				}
				else if (temp == ":") {
					tokenType = COLON;
				}
				else if (temp == ",") {
					tokenType = COMMA;
				}
				else {
					tokenType = ERROR1;
				}

				tempToken = new Token(lineShow, Word(temp, tokenType));
				TokenList.push_back(tempToken);
				c = fgetc(f);
			}
		}

		//判断未知符号
		else
		{
			string currentString = "" + c;
			Token* tempToken = new Token(lineShow, Word(currentString, ERROR1));
			TokenList.push_back(tempToken);
			c = fgetc(f);
		}
	}
	string temp(1, c);
	Token* tempToken = new Token(lineShow, Word(temp, ENDFILE1));
	TokenList.push_back(tempToken);
}

int main()
{
	LexicalAnalyzer lexicalanalyzer;
	lexicalanalyzer.getTokenList();
	int count = lexicalanalyzer.TokenList.size();
	for (int i = 0; i < count; i++)
	{
		cout<< "第"   << lexicalanalyzer.TokenList[i]->lineShow
			<< "行 <" << ha.at(lexicalanalyzer.TokenList[i]->word.Lex)
			<< ","    << lexicalanalyzer.TokenList[i]->word.Sem
			<< ">"    << endl;
	}
}