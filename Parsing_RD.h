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
	struct treeNode* child[3];		//�ӽڵ�
	struct treeNode* sibling;		//�ֵܽڵ�
	int lineno;						//Դ�����к�

	NodeKind nodekind;				/*��¼�﷨���ڵ����ͣ�ȡֵ ProK, PheadK, TypeK, VarK,
									  ProcDecK, StmLK, DecK, StmtK, ExpK, Ϊ�﷨���ڵ�����*/

	union
	{
		DecKind  dec;				/*��¼�﷨���ڵ���������ͣ��� nodekind = DecK ʱ��Ч��ȡ
									  ArrayK,CharK,lntegerK,RecordK,IdK, Ϊ�﷨���ڵ���������*/
		StmtKind stmt;				/*��¼�﷨���ڵ��������ͣ��� nodekind = StmtK ʱ��Ч��ȡֵ IfK, 
									  WhileK,AssignK,ReadK, WriteK, CallK,RetumK, Ϊ�﷨���ڵ��������*/
		ExpKind  exp;				/*��¼�﷨���ڵ�ı��ʽ���ͣ��� nodekind=ExpK ʱ��Ч��ȡֵ OpK, 
									  ConstK,varK, Ϊ�﷨���ڵ���ʽ����*/

	} kind;                         /* ��������     */

	int idnum;                      //��¼һ���ڵ��еı�־���ĸ���
	string name[10];				//�ڵ��б�־��������
	struct symbtable* table[10];	
	/*��¼�����������ڵ�Ϊ�������ͣ��������������ͱ�־����ʾʱ��Ч�������������*/

	struct
	{
		struct
		{
			int low;				// �����½�     
			int up;					// �����Ͻ�     
			DecKind   childtype;	// ����������� 
		}ArrayAttr;					// ��������     

		struct
		{
			ParamType  paramt;		/* ���̵Ĳ�������ֵΪö������valparamtype ���� varparamtype 
									   ��ʾ���̲�����ֵ�λ��Ǳ��*/
		}ProcAttr;					// ��������       

		struct
		{
			LexType op;				// ���ʽ�Ĳ�����
			int val;				// ���ʽ��ֵ
			VarKind  varkind;		// ���������    
			ExpType type;			// �������ͼ��  
		}ExpAttr;					// ���ʽ���� 

		string type_name;			//��¼�����������ڵ�Ϊ�������ͣ��������������ͱ�־����ʾʱ��Ч��

	} attr;							//��¼�﷨���ڵ���������
}TreeNode;

class RecursiveDescentParsing
{
public:
	void initial();							//ȫ�ֱ�����ʼ��
	void ReadNextToken();					//���ļ�
	void syntaxError(string errorMessage);	//����﷨����
	void match(LexType lt);					//ƥ��
	void fileClose();
	void printTree(TreeNode* tree);

	//1
	TreeNode* parse(void);
	TreeNode* Program(void);
	TreeNode* ProgramHead(void);
	TreeNode* DeclarePart(void);
	TreeNode* TypeDec(void);
	TreeNode* TypeDeclaration(void);
	TreeNode* TypeDecList(void);
	TreeNode* TypeDecMore(void);
	void TypeId(TreeNode* t);
	void TypeName(TreeNode* t);

	//11
	void BaseType(TreeNode* t);
	void StructureType(TreeNode* t);
	void ArrayType(TreeNode* t);
	void RecType(TreeNode* t);
	TreeNode* FieldDecList(void);
	TreeNode* FieldDecMore(void);
	void IdList(TreeNode* t);
	void IdMore(TreeNode* t);
	TreeNode* VarDec(void);
	TreeNode* VarDeclaration(void);

	//21
	TreeNode* VarDecList(void);
	TreeNode* VarDecMore(void);
	void VarIdList(TreeNode* t);
	void VarIdMore(TreeNode* t);
	TreeNode* ProcDec(void);
	TreeNode* ProcDeclaration(void);
	void ParamList(TreeNode* t);
	TreeNode* ParamDecList(void);
	TreeNode* ParamMore(void);
	TreeNode* Param(void);

	//31
	void FormList(TreeNode* t);
	void FidMore(TreeNode* t);
	TreeNode* ProcDecPart(void);
	TreeNode* ProcBody(void);
	TreeNode* ProgramBody(void);
	TreeNode* StmList(void);
	TreeNode* StmMore(void);
	TreeNode* Stm(void);
	TreeNode* AssCall(void);
	TreeNode* AssignmentRest(void);

	//41
	TreeNode* ConditionalStm(void);
	TreeNode* LoopStm(void);
	TreeNode* InputStm(void);
	TreeNode* OutputStm(void);
	TreeNode* ReturnStm(void);
	TreeNode* CallStmRest(void);
	TreeNode* ActParamList(void);
	TreeNode* ActParamMore(void);
	TreeNode* Exp(void);
	TreeNode* Simple_exp(void);

	//51
	TreeNode* Term(void);
	TreeNode* Factor(void);
	TreeNode* Variable(void);
	void VariMore(TreeNode* t);
	TreeNode* Fieldvar(void);
	void FieldvarMore(TreeNode* t);
};