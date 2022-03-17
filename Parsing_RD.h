#pragma once
#include <iostream>
#include<fstream>
#include<string>
#include<unordered_map>
#include<algorithm>
#include<set>
#include<vector>
#include<iomanip>
#include<sstream>
#include"LexicalAnalyzer.h"
using namespace std;

/******************************************************
 ******************   �﷨������   ********************
 ******************************************************/

 /*�﷨�����ڵ�ProK,����ͷ���PheadK���������ͽڵ�DecK,
   ��־�ӽ�㶼�����������Ľ��TypeK,��־�ӽ�㶼�Ǳ��������Ľ��VarK,
   �����������FuncDecK,������нڵ�StmLK,����������StmtK,
   ���ʽ���ExpK*/
typedef enum { ProK, PheadK, DecK, TypeK, VarK, ProcDecK, StmLK, StmtK, ExpK }
NodeKind;

/*��������Deckind ���͵�ö�ٶ��壺
  ��������ArrayK,�ַ�����CharK,
  ��������IntegerK,��¼����RecordK,
  �����ͱ�ʶ����Ϊ���͵�IdK*/
typedef enum { ArrayK, CharK, IntegerK, RecordK, IdK }  DecKind;

/* �������StmtKind���͵�ö�ٶ���:			*
 * �ж�����IfK,ѭ������WhileK				*
 * ��ֵ����AssignK,������ReadK              *
 * д����WriteK��������������CallK          */
typedef enum { IfK, WhileK, AssignK, ReadK, WriteK, CallK, ReturnK } StmtKind;


/* ���ʽ����ExpKind���͵�ö�ٶ���:         *
 * ��������OpK,��������ConstK,��������VarK  */
typedef enum { OpK, ConstK, VariK } ExpKind;


/* ��������VarKind���͵�ö�ٶ���:           *
 * ��ʶ��IdV,�����ԱArrayMembV,���ԱFieldMembV*/
typedef enum { IdV, ArrayMembV, FieldMembV } VarKind;


/* ���ͼ��ExpType���͵�ö�ٶ���:           *
 * ��Void,��������Integer,�ַ�����Char      */
typedef enum { Void, Integer, Boolean } ExpType;

/* ��������ParamType���͵�ö�ٶ��壺        *
 * ֵ��valparamType,���varparamType        */
typedef enum { valparamType, varparamType } ParamType;

/*��ǰ�������ű�ṹ*/
struct symbtable;

/********** �﷨���ڵ�treeNode���� *********/
typedef struct treeNode

{
	struct treeNode* child[3];		/* �ӽڵ�ָ��	*/
	struct treeNode* sibling;		/* �ֵܽڵ�ָ��	*/
	int lineno;						/* Դ�����к�	*/
	NodeKind nodekind;				/* �ڵ�����		*/
	union
	{
		DecKind  dec;
		StmtKind stmt;
		ExpKind  exp;
	} kind;                       /* ��������     */

	int idnum;                    /* ��ͬ���͵ı������� */

	string name[10];            /* ��ʶ��������  */

	struct symbtable* table[10];  /* ���־����Ӧ�ķ��ű��ַ������������׶�����*/

	struct
	{
		struct
		{
			int low;              /* �����½�     */
			int up;               /* �����Ͻ�     */
			DecKind   childtype;  /* ����������� */
		}ArrayAttr;               /* ��������     */

		struct
		{
			ParamType  paramt;     /* ���̵Ĳ������� */
		}ProcAttr;                 /* ��������       */

		struct
		{
			LexType op;           /* ���ʽ�Ĳ�����*/
			int val;		      /* ���ʽ��ֵ	   */
			VarKind  varkind;     /* ���������    */
			ExpType type;         /* �������ͼ��  */
		}ExpAttr;	              /* ���ʽ����    */

		string type_name;       /* �������Ǳ�ʶ�� */

	} attr;                       /* ����	        */
}TreeNode;

class RecursiveDescentParsing
{
public:
	void initial();							//ȫ�ֱ�����ʼ��
	void ReadNextToken();					//���ļ�
	void syntaxError(string errorMessage);	//����﷨����
	void match(LexType lt);

	TreeNode* parse(void);
	TreeNode* Program(void);
	TreeNode* ProgramHead(void);
	TreeNode* DeclarePart(void);
	TreeNode* TypeDec(void);
	TreeNode* TypeDecList(void);
	TreeNode* TypeDecMore(void);
	void TypeId(TreeNode* t);
	void TypeName(TreeNode* t);
	void BaseType(TreeNode* t);

};