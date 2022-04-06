#include <iostream>
#include<fstream>
#include<string>
#include<unordered_map>
#include<algorithm>
#include<set>
#include<vector>
#include <iomanip>
#include <sstream>
#include <graphics.h>
#include"LexicalAnalyzer.h"
#include"Parsing_RD.h"
#include"SemanticAnalysis.h"
using namespace std;
int main()
{
	/*======�ʷ�����=====*/
	LexicalAnalyzer lexicalanalyzer;
	lexicalanalyzer.getTokenList();
	int count = lexicalanalyzer.TokenList.size();

	//for (int i = 0; i < count; i++)
	//{
	//	cout<< "��"   << lexicalanalyzer.TokenList[i]->lineShow
	//		<< "�� <" << ha.at(lexicalanalyzer.TokenList[i]->word.Lex)
	//		<< ","    << lexicalanalyzer.TokenList[i]->word.Sem
	//		<< ">"    << endl;
	//}

	/*=====�﷨����=====*/
	ofstream file;
	file.open("tokenList.txt");
	for (int i = 0; i < count; i++)
	{
		file << lexicalanalyzer.TokenList[i]->lineShow
			<< ' ' << lexicalanalyzer.TokenList[i]->word.Lex
			<< ' ' << lexicalanalyzer.TokenList[i]->word.Sem
			<< endl;
	}
	file.close();

	RecursiveDescentParsing rd;
	rd.initial();
	TreeNode* root;
	root = rd.parse();

	{
		initgraph(1300, 640);   // ����ͼ�ν���
		for (int y = 0; y <= 640; y++)
		{
			setcolor(RGB(255, 255, 255));
			line(0, y, 1300, y);
		}						//��ɫ����
		rd.printTree(root);
		saveimage(_T("treeFile.bmp"));
	}
	closegraph();
	rd.fileClose();

	/*=====�������=====*/
	SemanticAnalysis sa;
	sa.initial();
	sa.analyze(root);
	sa.PrintSymbTable();
	sa.fileClose();

	cout << "���гɹ�" << endl;
}