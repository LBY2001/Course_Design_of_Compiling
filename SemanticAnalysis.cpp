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
#include"SemanticAnalysis.h"
#include"Parsing_RD.h"


SymbTable* scope[1000];			//scopeջ
int scopeLevel;					//scope����
int Off;						//ͬ��ı���ƫ����
ofstream errorFile;				//���������Ϣ�ļ�
ofstream tableFile;				//������ű��ļ�
bool hasError;
TypeIR* intPtr;					//ָ���������͵��ڲ���ʾ
TypeIR* charPtr;				//ָ���ַ����͵��ڲ���ʾ
TypeIR* boolPtr;				//ָ�򲼶����͵��ڲ���ʾ



void SemanticAnalysis::initial()
{
	for (int i = 0; i < 1000; i++)
		scope[i] = NULL;
	scopeLevel = -1;
	hasError = false;
	errorFile.open("parseError.txt", ios::out | ios::app);
	errorFile.open("symbTable.txt");

	intPtr = NewTy(intTy);
	charPtr = NewTy(charTy);
	boolPtr = NewTy(boolTy);
}

void SemanticAnalysis::fileClose()
{
	errorFile.close();
}

//����������
void SemanticAnalysis::semanticError(int line, string errorMessage)
{
	errorFile << "�������, line " << line << " : " << errorMessage << endl;
	hasError = true;
}

//�½��շ��ű�
SymbTable* SemanticAnalysis::NewTable(void)
{
	SymbTable* table = new SymbTable;
	if (table == NULL)
	{
		errorFile << "�ڴ���� " << endl;
		hasError = true;
		return NULL;
	}

	table->next = NULL;
	table->attrIR.kind = typeKind;
	table->attrIR.idtype = NULL;
	table->next = NULL;
	table->attrIR.More.VarAttr.isParam = false;
	return table;
}

TypeIR* SemanticAnalysis::NewTy(TypeKind kind)
{
	TypeIR* table = new TypeIR;
	if (table == NULL)
	{
		errorFile << "�ڴ���� " << endl;
		hasError = true;
	}
	else
		switch (kind)
		{
			case intTy:
			case charTy:
			case boolTy:
				table->kind = kind;
				table->size = 1;
				break;
			case arrayTy:
				table->kind = arrayTy;
				table->More.ArrayAttr.indexTy = NULL;
				table->More.ArrayAttr.elemTy = NULL;
				break;
			case recordTy:
				table->kind = recordTy;
				table->More.body = NULL;
				break;
		}
	return table;
}

//�����շ��ű�table
void SemanticAnalysis::CreatTable(void)
{
	Off = 7;						//�̲�P161��Ϊ����7
	scopeLevel++;					//������1
	scope[scopeLevel] = NULL;		//��scopeջ
}

void SemanticAnalysis::DestroyTable()
{
	scopeLevel--;
}

//�Ǽ�
bool SemanticAnalysis::Enter(string id, AttributeIR* attribP, SymbTable** entry)
{
	bool present = false;
	SymbTable* pNow = scope[scopeLevel];
	SymbTable* pLast = scope[scopeLevel];

	//�ձ������ظ����壬ֱ�ӽ������ű���
	if (scope[scopeLevel] == NULL)
	{
		pNow = NewTable();
		scope[scopeLevel] = pNow;
	}
	
	//�����ж��ظ��������
	//���ҵ���
	else
	{
		//�ڸò���ű��ڼ���ظ��������,���¼��present
		while ((pNow != NULL) && (present != true))
		{
			pLast = pNow;
			if (id == pNow->idName)
			{
				errorFile << "��ʶ���ظ����� " << endl;
				hasError = true;
				present = true;
			}
			else
				pNow = pLast->next;
		}

		if (present == false)
		{
			pNow = NewTable();
			pLast->next = pNow;
		}
	}

	//����ʶ���ظ�������ֱ�ӷ���
	if (present == true)
		return true;

	//����ʶ���������ԵǼǵ�����
	pNow->idName = id;
	pNow->attrIR.idtype = attribP->idtype;
	pNow->attrIR.kind = attribP->kind;
	switch (attribP->kind)
	{
		case  typeKind: 
			break;
		case  varKind:
			pNow->attrIR.More.VarAttr.level = attribP->More.VarAttr.level;
			pNow->attrIR.More.VarAttr.off = attribP->More.VarAttr.off;
			pNow->attrIR.More.VarAttr.access = attribP->More.VarAttr.access;
			break;
		case  procKind:
			pNow->attrIR.More.ProcAttr.level = attribP->More.ProcAttr.level;
			pNow->attrIR.More.ProcAttr.param = attribP->More.ProcAttr.param;
			break;
		default:
			break;
	}
	if (entry != NULL)
		(*entry) = pNow;
	return present;
}

//����ȫ�����ű�
bool SemanticAnalysis::FindEntry(string id, SymbTable** entry)
{
	bool present = false;
	int level = scopeLevel;		//��ʱ��¼����

	SymbTable* findentry = scope[level];
	while ((level != -1) && (present != true))
	{
		while ((findentry != NULL) && (present != true))
		{
			if (id == findentry->idName)
				present = true;
			else
				findentry = findentry->next;
		}
		//������һ���ֲ�������
		if (present != true)
		{
			level--;
			findentry = scope[level];
		}
	}

	//?
	if (present != true)
		(*entry) = NULL;
	else
		(*entry) = findentry;

	return present;
}

bool SemanticAnalysis::FindField(string Id, fieldChain* head, fieldChain** Entry)
{
	bool present = false;
	fieldChain* pNow = head;
	while ((pNow != NULL) && (present == false))
	{
		if (pNow->id == Id)
		{
			present = true;
			if (Entry != NULL)
				(*Entry) = pNow;
		}
		else  
			pNow = pNow->Next;
	}
	return present;
}

//��ӡ���ű�
void SemanticAnalysis::PrintSymbTable()
{
	int level = 0;
	while (scope[level] != NULL)
	{
		SymbTable* t = scope[level];
		tableFile << "==========��" << level << "����ű�==========" << endl;
		while (t != NULL)
		{
			//��ʶ����
			tableFile << t->idName << ":  ";
			AttributeIR* Attrib = &(t->attrIR);

			//��ʶ������
			//���ǹ��̱�ʶ��
			if (Attrib->idtype != NULL)
				switch (Attrib->idtype->kind)
				{
					case  intTy:  
						tableFile <<"intTy  "; 
						break;
					case  charTy:
						tableFile << "charTy  ";
						break;
					case  arrayTy:
						tableFile << "arrayTy  ";
						break;
					case  recordTy:
						tableFile << "recordTy  ";
						break;
					default:
						tableFile << "Error  ";
						break;
				}

			//��ʶ����𣬼������Ϣ
			switch (Attrib->kind)
			{
				case  typeKind:
					tableFile << "typeKind  ";
					break;
				case  varKind:
					tableFile << "varKind  ";
					tableFile << "Level: " << Attrib->More.VarAttr.level << "  ";
					tableFile << "Offset: " << Attrib->More.VarAttr.off << "  ";
					switch (Attrib->More.VarAttr.access)
					{
						case  dir:  
							tableFile << "dir  ";
							break;
						case  indir: 
							tableFile << "indir  ";
							break;
						default:
							tableFile << "ErrorKind  ";
							break;
					}
					break;
				case  procKind:
					tableFile << "funcKind  ";
					tableFile << "Level: " << Attrib->More.ProcAttr.level << "  ";
					tableFile << "nOff: " << Attrib->More.ProcAttr.nOff << "  ";
					break;
				default:
					tableFile << "Error  ";
			}
			tableFile << endl;
			t = t->next;
		}
		level++;
	}
}

void SemanticAnalysis::analyze(TreeNode* currentP)
{
	TreeNode* p = NULL;
	/*�������ű�*/
	CreatTable();

	//ѭ���������������������
	p = currentP->child[1];
	while (p != NULL)
	{
		switch (p->nodekind)
		{
			case  TypeK:
				TypeDecPart(p->child[0]);
				break;
			case  VarK:
				varDecList(p->child[0]);
				break;
			case  ProcDecK:
				procDecPart(p);
				break;
			default:
				semanticError(p->lineno, "���������쳣");
				break;
		}
		p = p->sibling;
	}

	//������������
	currentP = currentP->child[2];
	if (currentP->nodekind == StmLK)
		Body(currentP);

	//�������ű�
	if (scopeLevel != -1)
		DestroyTable();
}

TypeIR* SemanticAnalysis::TypeProcess(TreeNode* t, DecKind deckind)
{
	TypeIR* Ptr = NULL;
	switch (deckind)
	{
		case IdK:
			Ptr = nameType(t); 
			break;
		case ArrayK:
			Ptr = arrayType(t); 
			break;
		case RecordK:
			Ptr = recordType(t); 
			break;
		case IntegerK:
			Ptr = intPtr; 
			break;
		case CharK:
			Ptr = charPtr; 
			break;
	}
	return Ptr;
}

TypeIR* SemanticAnalysis::nameType(TreeNode* t)
{
	TypeIR* Ptr = NULL;
	SymbTable* entry = NULL;

	//���ű����ҵ�������
	if (FindEntry(t->attr.type_name, &entry))
	{
		//���ñ�ʶ���Ƿ�Ϊ���ͱ�ʶ��
		if (entry->attrIR.kind != typeKind)
			semanticError(t->lineno, "�����ͱ�ʶ����");
		else
			Ptr = entry->attrIR.idtype;
	}
	//δ�ҵ�������
	else
	{
		semanticError(t->lineno, "����������");
	}
	return Ptr;
}

TypeIR* SemanticAnalysis::arrayType(TreeNode* t)
{
	TypeIR* present = NULL;
	if (t->attr.ArrayAttr.low > t->attr.ArrayAttr.up)
	{
		semanticError(t->lineno, "����Խ��");
		hasError = true;
	}
	else
	{
		//present->kindҲ���⸳ֵ
		present = NewTy(arrayTy);
		present->size = (t->attr.ArrayAttr.up - t->attr.ArrayAttr.low + 1) * present->size;
		//�������±����ͣ�Ӧ����ֻ��������
		present->More.ArrayAttr.indexTy = TypeProcess(t, IntegerK);
		present->More.ArrayAttr.elemTy = TypeProcess(t, t->attr.ArrayAttr.childtype);
		present->More.ArrayAttr.low = t->attr.ArrayAttr.low;
		present->More.ArrayAttr.up = t->attr.ArrayAttr.up;
	}
	return present;
}

TypeIR* SemanticAnalysis::recordType(TreeNode* t)
{
	TypeIR* Ptr = NewTy(recordTy);
	t = t->child[0];

	fieldChain* pLast = NULL;	//�����ڶ��id����Ϊ��һ���ڵ�ָ��
	fieldChain* pNow = NULL;	//�����ڶ��id����Ϊ����ָ��
	fieldChain* pTemp = NULL;
	while (t != NULL)
	{
		for (int i = 0; i < t->idnum; i++)
		{
			pNow = NewBody();
			if (pTemp == NULL)
			{
				pTemp = pNow;
				pLast = pNow;
			}

			pNow->id = t->name[i];
			pNow->UnitType = TypeProcess(t, t->kind.dec);
			pNow->Next = NULL;

			//pLast!=pNow����ô��ָ�����
			if (pNow != pLast)
			{
				//����������ĵ�Ԫoff
				pNow->off = pLast->off + pLast->UnitType->size;
				pLast->Next = pNow;
				pLast = pNow;
			}
		}
		t = t->sibling;
	}
	
}

//��鱾�������������Ƿ����ظ��������
void SemanticAnalysis::TypeDecPart(TreeNode* t)
{
	AttributeIR attrI;
	SymbTable* entry = NULL;
	//KindΪ��������
	attrI.kind = typeKind;
	while (t != NULL)
	{
		if (Enter(t->name[0], &attrI, &entry) != false)
		{
			string temp = t->name[0].c_str();
			semanticError(t->lineno, temp + "�ظ���������");
			entry = NULL;
		}
		else
			entry->attrIR.idtype = TypeProcess(t, t->kind.dec);
		t = t->sibling;
	}
}

//��鱾������������Ƿ����ظ��������
void SemanticAnalysis::varDecList(TreeNode* t)
{
	AttributeIR attrIr;
	SymbTable* entry = NULL;
	while (t != NULL)
	{
		//KindΪ��������
		attrIr.kind = varKind;
		//ѭ������ͬһ���ڵ��id���������ʹ�����
		for (int i = 0; i < t->idnum; i++)
		{
			attrIr.idtype = TypeProcess(t, t->kind.dec);

			//�ж��Ƿ�Ϊֱ�ӱ���
			if (t->attr.ProcAttr.paramt == valparamType)
			{
				//ֱ�ӱ���
				attrIr.More.VarAttr.access = dir;
				attrIr.More.VarAttr.level = scopeLevel;
				attrIr.More.VarAttr.off = Off;
				Off = Off + attrIr.idtype->size;
			}//ƫ�Ƽӱ������͵�size

			else
			{
				//��ӱ���(���)
				attrIr.More.VarAttr.access = indir;
				attrIr.More.VarAttr.level = scopeLevel;
				attrIr.More.VarAttr.off = Off;
				Off = Off + 1;
			}//P168 ��ӱ�����ƫ�Ƽ�1
			

			//�ǼǸñ��������Լ�����,���ж��Ƿ��ظ�����
			if (Enter(t->name[i], &attrIr, &entry) != false)
			{
				string temp = t->name[i].c_str();
				semanticError(t->lineno, temp + "�ظ���������");
				entry = NULL;
			}
			else
				t->table[i] = entry;
				//��¼������
		}
		t = t->sibling;
	}

}

//�ڵ�ǰ����ű���д���̷��ű����ԣ����²���ű���д�βα�ʶ������
void SemanticAnalysis::procDecPart(TreeNode* t)
{
	SymbTable* entry = HeadProcess(t);
	t = t->child[1];
	while (t != NULL)
	{
		//������������
		switch (t->nodekind)
		{
			case  TypeK:     
				TypeDecPart(t->child[0]);
				break;
			case  VarK:
				varDecList(t->child[0]);
				break;
			case  ProcDecK: 
				procDecPart(t->child[0]);
				break;
			default:
				semanticError(t->lineno, "���������쳣");
				break;
		}
		t = t->sibling;
	}

	//������ǰscope
	DestroyTable();
}

//��������ͷ��������
SymbTable* SemanticAnalysis::HeadProcess(TreeNode* t)
{
	AttributeIR attrIr;
	SymbTable* entry = NULL;

	//��д����
	attrIr.idtype = NULL;
	attrIr.kind = procKind;
	attrIr.More.ProcAttr.level = scopeLevel + 1;

	//�ǼǱ����
	bool temp = Enter(t->name[0], &attrIr, &entry);
	t->table[0] = entry;

	//�����βδ�����
	entry->attrIR.More.ProcAttr.param = ParaDecList(t);

	return entry;
}

//?
ParamTable* SemanticAnalysis::ParaDecList(TreeNode* t)
{
	TreeNode* p = NULL;
	ParamTable* result = NULL;
	if (t->child[0] != NULL)
	{
		p = t->child[0];

		//�����¾ֲ�����
		CreatTable();

		//���ú���varDecPart
		varDecList(p);

		//����Ҫѭ����
		//�����βη��ű�
		SymbTable* temp = scope[scopeLevel];
		ParamTable* Ptemp;//���ű�
		Ptemp = NewParam();
		Ptemp->entry = temp;
		Ptemp->next = NULL;
		result = Ptemp;
	}

	return result;
}

void SemanticAnalysis::Body(TreeNode* t)
{
	TreeNode* p = t->child[0];
	while (p != NULL)
	{
		statement(p);
		p = p->sibling;
	}
}

void SemanticAnalysis::statement(TreeNode* t)
{
	switch (t->kind.stmt)
	{
		case IfK:			
			ifstatment(t);
			break;
		case WhileK:
			whilestatement(t);
			break;
		case AssignK:
			assignstatement(t);
			break;
		case ReadK:
			readstatement(t);
			break;
		case WriteK:
			writestatement(t);
			break;
		case CallK:
			callstatement(t);
			break;
		case ReturnK:
			returnstatement(t);
			break;
		default:
			semanticError(t->lineno, "��Ч�������");
			break;
	}
}

//���������������������ԣ�����ʽ�����͡�Ekind������ʾʵ���Ǳ�λ���ֵ��
TypeIR* SemanticAnalysis::Expr(TreeNode* t, AccessKind* Ekind)
{
	//ָ����ʽ�����ڲ���ʾ��ָ��
	TypeIR* Eptr = NULL;
	SymbTable* entry;
	switch (t->kind.exp)
	{
		//��������
		case ConstK:
			Eptr = TypeProcess(t, IntegerK);
			Eptr->kind = intTy;
			(*Ekind) = dir;   //ֱ�ӱ���
			break;

		//��������
		case VariK:
			//Ѱ������
			if (t->child[0] == NULL)
			{
				bool tempb = FindEntry(t->name[0], &entry);
				t->table[0] = entry;
				if (tempb == true)
				{   
					//�ж��Ƿ�Ϊ��ӱ���
					if (FindAttr(entry).kind == varKind)
					{
						Eptr = entry->attrIR.idtype;
						*Ekind = indir;  //��ӱ���
					}
					else
					{
						string temp = t->name[0].c_str();
						semanticError(t->lineno, temp + "���Ǳ���");
						Eptr = NULL;
					}
				}
				else
				{
					string temp = t->name[0].c_str();
					semanticError(t->lineno, temp + "����ȱʧ");
				}

			}
			//��������������
			else
			{
				//�������
				if (t->attr.ExpAttr.varkind == ArrayMembV)
					Eptr = arrayVar(t);
				//�����
				else if (t->attr.ExpAttr.varkind == FieldMembV)
					Eptr = recordVar(t);
				else
				{
					string temp = t->name[0].c_str();
					semanticError(t->lineno, temp + "��������������");
				}
			}
			break;

		//������
		case OpK:
			TypeIR* op1 = NULL;	//������1
			TypeIR* op2 = NULL;	//������2
			op1 = Expr(t->child[0], NULL);
			op2 = Expr(t->child[1], NULL);

			if (op1 == NULL || op2 == NULL)
			{
				semanticError(t->lineno, "��������������");
				return NULL;
			}

			present = Compat(op1, op2);
			if (present != false)
			{
				switch (t->attr.ExpAttr.op)
				{
					case LT:
					case EQ:
						Eptr = boolPtr;
						break;  /*�������ʽ*/
					case PLUS:
					case MINUS:
					case TIMES:
					case OVER:
						Eptr = intPtr;
						break;  /*�������ʽ*/
				}
				if (Ekind != NULL)
					(*Ekind) = dir; /*ֱ�ӱ���*/
			}
			else
				semanticError(t->lineno, "���������Ͳ�����");
			break;
	}
	return Eptr;
}



