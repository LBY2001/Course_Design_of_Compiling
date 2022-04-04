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
#include"Parsing_RD.h"

using namespace std;

//��ʶ��������
typedef  enum { typeKind, varKind, procKind }IdKind;

//���������dir��ֱ�ӱ���(ֵ��)��indir��ʾ��ӱ���(���)
typedef  enum { dir, indir }AccessKind;

//�βα�Ľṹ����
typedef struct  paramTable
{
	struct symbtable* entry;		//ָ����β����ڷ��ű��еĵ�ַ���
	struct paramTable* next;		//ָ����һ���β�
}ParamTable;

//��ʶ���ṹ
typedef struct
{
	struct typeIR* idtype;			//ָ���ʶ���������ڲ���ʾ
	IdKind kind;					//��ʶ��������
	union
	{
		struct
		{
			AccessKind access;		//�ж��Ǳ�λ���ֵ��
			int level;
			int off;
			bool isParam;			//�ж��ǲ���������ͨ����

		}VarAttr;					//������ʶ��������
		struct
		{
			int level;				/*�ù��̵Ĳ���*/
			ParamTable* param;		/*������*/
			int mOff;				/*���̻��¼�Ĵ�С*/
			int nOff;  				/*sp��display���ƫ����*/
			int procEntry;			/*���̵���ڵ�ַ*/
			int codeEntry;			/*������ڱ��,�����м��������*/
		}ProcAttr;					/*��������ʶ��������*/
	}More;							/*��ʶ���Ĳ�ͬ�����в�ͬ������*/
}AttributeIR;

//���ű�Ľṹ����
typedef struct  symbtable
{
	string idName;
	AttributeIR  attrIR;
	struct symbtable* next;

}SymbTable;

/* �����ڲ���ʾ */
//���͵�ö�ٶ���
typedef  enum { intTy, charTy, arrayTy, recordTy, boolTy }TypeKind;

//�����͵�Ԫ�ṹ����
typedef struct fieldchain
{
	string id;					//������
	int    off;                 //���ڼ�¼�е�ƫ��
	struct typeIR* UnitType;	//���г�Ա������
	struct fieldchain* Next;
}fieldChain;


//���͵��ڲ��ṹ����
typedef struct typeIR
{
	int	size;   /*������ռ�ռ��С*/
	TypeKind kind;
	union
	{
		struct
		{
			struct typeIR* indexTy;	//ָ�������±�����
			struct typeIR* elemTy;	//ָ������Ԫ������
			int low;     //�½�
			int up;      //�Ͻ�
		}ArrayAttr;
		fieldChain* body;  //ָ�����Ա������
	} More;
}TypeIR;

class SemanticAnalysis
{
public:
	//��������
	void initial();				//ȫ�ֱ�����ʼ��
	void fileClose();			//�ļ��ر�
	void semanticError(int line, string errorMessage);
	SymbTable* NewTable(void);	//�½��շ��ű�
	TypeIR* NewTy(TypeKind kind);//�½������ڲ���ʾ
	fieldChain* NewBody();		//�½���
	ParamTable* NewParam();		//�½��β����� 

	//���ű�ʵ��
	void CreatTable(void);		//�������ű�
	void DestroyTable();		//�������ű�
	bool Enter(string id, AttributeIR* AttribP, SymbTable** Entry);
								//�ǼǱ�ʶ�������Ե����ű�
	bool FindEntry(string id, SymbTable** entry);
	bool FindField(string Id, fieldChain* head, fieldChain** Entry);
	void PrintSymbTable();		//��ӡ���ű�


	//���������Ϣ
	void analyze(TreeNode* currentP);	//�������������
	TypeIR* TypeProcess(TreeNode* t, DecKind deckind);
										//���ͷ�������
	TypeIR* nameType(TreeNode* t);		//�Զ��������ڲ��ṹ����
	TypeIR* arrayType(TreeNode* t);		//��������
	TypeIR* recordType(TreeNode* t);	//��¼����
	void TypeDecPart(TreeNode* t);		//�����������ַ���������
	void varDecList(TreeNode* t);		//�����������ַ���������
	void procDecPart(TreeNode* t);		//�����������ַ���������
	SymbTable* HeadProcess(TreeNode* t);//��������ͷ��������
	ParamTable* ParaDecList(TreeNode* t);//�βη���������
	void Body(TreeNode* t);				//ִ���岿�ַ���������
	void statement(TreeNode* t);		//������з���������
	TypeIR* Expr(TreeNode* t, AccessKind* Ekind);
										//���ʽ����������
	TypeIR* arrayVar(TreeNode* t);		//��������Ĵ����������
	TypeIR* recordVar(TreeNode* t);		//��¼������������ķ���������
	void assignstatement(TreeNode* t);	//��ֵ����������
	void callstatement(TreeNode* t);	//���̵���������������
	void ifstatment(TreeNode* t);		//����������������
	void whilestatement(TreeNode* t);	//ѭ��������������
	void readstatement(TreeNode* t);	//��������������
	void writestatement(TreeNode* t);	//д������������
	void returnstatement(TreeNode* t);	//����������������
};