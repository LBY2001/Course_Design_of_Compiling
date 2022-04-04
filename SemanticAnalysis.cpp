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

fieldChain* SemanticAnalysis::NewBody()
{
	fieldChain* Ptr = new fieldChain;
	if (Ptr == NULL)
	{
		errorFile << "�ڴ���� " << endl;
		hasError = true;
	}
	else
	{
		Ptr->Next = NULL;
		Ptr->off = 0;
		Ptr->UnitType = NULL;
	}
	return Ptr;
}

ParamTable* SemanticAnalysis::NewParam()
{
	ParamTable* Ptr = new ParamTable;
	if (Ptr == NULL)
	{
		errorFile << "�ڴ���� " << endl;
		hasError = true;
	}
	else
	{
		Ptr->entry = NULL;
		Ptr->next = NULL;
	}
	return Ptr;
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
	return Ptr;
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
			if (Ekind != NULL)
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
					if (entry->attrIR.kind == varKind)
					{
						Eptr = entry->attrIR.idtype;
						if (Ekind != NULL)
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
				//������� Var = Var0[E]
				if (t->attr.ExpAttr.varkind == ArrayMembV)
					Eptr = arrayVar(t);
				//����� Var = Var0.id
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

			if (op1 == op2)
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

//��������Ĵ����������
TypeIR* SemanticAnalysis::arrayVar(TreeNode* t)
{
	TypeIR* Eptr = NULL;
	SymbTable* entry = NULL;
	if (FindEntry(t->name[0], &entry))
	{
		string temp = t->name[0].c_str();
		semanticError(t->lineno, temp + "δ�ҵ��˱�ʶ��");
		entry = NULL;
	}
	else
	{
		if (entry->attrIR.kind != varKind)
		{
			string temp = t->name[0].c_str();
			semanticError(t->lineno, temp + "���Ǳ���");
		}
		else
		{
			if (entry->attrIR.idtype != NULL)
			{
				string temp = t->name[0].c_str();
				semanticError(t->lineno, temp + "�����������");
			}
			else
			{
				//�����±�����
				TypeIR* temp1 = NULL;
				TypeIR* temp2 = NULL;
				temp1 = entry->attrIR.idtype->More.ArrayAttr.indexTy;
				temp2 = Expr(t->child[0], NULL);
				if (temp1 == NULL || temp2 == NULL)
					return NULL;

				if (temp1 == temp2)
					Eptr = entry->attrIR.idtype->More.ArrayAttr.elemTy;
				else
				{
					string temp = t->name[0].c_str();
					semanticError(t->lineno, temp + "���±����Ͳ����");
				}
			}
		}
	}
	return Eptr;
}

//��¼������������ķ���������
TypeIR* SemanticAnalysis::recordVar(TreeNode* t)
{
	TypeIR* Eptr = NULL;
	SymbTable* entry = NULL;
	if (FindEntry(t->name[0], &entry) == false)
	{
		string temp = t->name[0].c_str();
		semanticError(t->lineno, temp + "δ�ҵ��˱�ʶ��");
	}
	else
	{
		if (entry->attrIR.kind != varKind)
		{
			string temp = t->name[0].c_str();
			semanticError(t->lineno, temp + "���Ǳ���");
		}
		else
		{
			if (entry->attrIR.idtype->kind != recordTy)
			{
				string temp = t->name[0].c_str();
				semanticError(t->lineno, temp + "���Ǳ���");
			}
			//���Ϸ�����,��id���Ƿ�������
			else
			{
				fieldChain* tempF = NULL;
				tempF = entry->attrIR.idtype->More.body;
				while (tempF != NULL)
				{
					//����Ȳ���һ��
					if (t->child[0]->name[0] != tempF->id)
						tempF = tempF->Next;
					else
						Eptr = tempF->UnitType;
				}

				
				if (tempF == NULL)
				{
					string temp = t->name[0].c_str();
					semanticError(t->lineno, temp + "���ǺϷ�����");
				}
				else
				{
					//?�������
					if (t->child[0]->child != NULL)
						Eptr = arrayVar(t->child[0]);
				}
			}
		}
	}
	return Eptr;
}

//��ֵ������
void SemanticAnalysis::assignstatement(TreeNode* t)
{
	TypeIR* Eptr = NULL;
	SymbTable* entry = NULL;
	TreeNode* child1 = t->child[0];
	TreeNode* child2 = t->child[1];

	if (child1->child[0] == NULL)
	{
		if (FindEntry(t->name[0], &entry) == false)
		{
			string temp = t->name[0].c_str();
			semanticError(t->lineno, temp + "δ�ҵ��˱�ʶ��");
			entry = NULL;
		}
		else
		{
			if (entry->attrIR.kind != varKind)
			{
				string temp = t->name[0].c_str();
				semanticError(t->lineno, temp + "���Ǳ���");
			}
			else
			{
				Eptr = entry->attrIR.idtype;
				child1->table[0] = entry;
			}
		}
	}
	else
	{
		if (child1->attr.ExpAttr.varkind == ArrayMembV)
			Eptr = arrayVar(child1);
		//? child1->attr.ExpAttr.varkind==FieldMembV �Ƿ��ж�
		else
			Eptr = recordVar(child1);
	}

	//��鸳ֵ�������Ƿ�ȼ�
	if (Eptr != NULL)
		if (Expr(child2, NULL) != Eptr)
			semanticError(t->lineno, "�Ⱥ����಻�ȼ�");
}

//����������
void SemanticAnalysis::callstatement(TreeNode* t)
{
	SymbTable* entry = NULL;
	bool temp = FindEntry(t->child[0]->name[0], &entry);
	t->child[0]->table[0] = entry;

	if (temp == false)
	{
		string temp = t->name[0].c_str();
		semanticError(t->lineno, temp + "δ�ҵ��˺���");
	}
	else
	{
		if (entry->attrIR.kind != procKind)
		{
			string temp = t->name[0].c_str();
			semanticError(t->lineno, temp + "���Ǻ�����");
		}
		//�ж��β�ʵ�ζ�Ӧ
		else
		{
			//������ָ��
			ParamTable* tempTable = entry->attrIR.More.ProcAttr.param;
			//ʵ��ָ��
			TreeNode* pNode = t->child[1];

			while (tempTable != NULL && pNode != NULL)
			{
				AccessKind tempA;
				TypeIR* tempT = Expr(pNode, &tempA);

				if ((tempTable->entry->attrIR.More.VarAttr.access == indir) && (tempA == dir))
					semanticError(t->lineno, "ֵ�����Ͳ���ʹ�ñ��");
				else
					if ((tempTable->entry->attrIR.idtype) != tempT)
						semanticError(t->lineno, "�������Ͳ�ƥ��");
				
				pNode = pNode->sibling;
				tempTable = tempTable->next;
			}

			//����������ƥ��
			if (tempTable != NULL || pNode != NULL)
				semanticError(t->lineno, "����������ƥ��");
		}

	}
}

//����������������
void SemanticAnalysis::ifstatment(TreeNode* t)
{
	TypeIR* Etp = Expr(t->child[0], NULL);

	//�������ʽ��bool��
	if (Etp->kind != boolTy)
		semanticError(t->lineno, "���ʽ��bool��");
	else
	{
		//then���
		TreeNode* p = t->child[1];
		while (p != NULL)
		{
			statement(p);
			p = p->sibling;
		}

		//else���
		p = t->child[2];
		while (p != NULL)
		{
			statement(p);
			p = p->sibling;
		}
	}
}

//ѭ��������������
void SemanticAnalysis::whilestatement(TreeNode* t)
{
	TypeIR* Etp = Expr(t->child[0], NULL);

	//�������ʽ��bool��
	if (Etp->kind != boolTy)
		semanticError(t->lineno, "���ʽ��bool��");
	else
	{
		//�������
		TreeNode* p = t->child[1];
		while (p != NULL)
		{
			statement(p);
			p = p->sibling;
		}
	}
}

//��������������
void SemanticAnalysis::readstatement(TreeNode* t)
{
	SymbTable* entry = NULL;
	if (FindEntry(t->name[0], &entry) == false)
	{
		string temp = t->name[0].c_str();
		semanticError(t->lineno, temp + "δ�ҵ��˱�ʶ��");
	}
	else
		if (entry->attrIR.kind != varKind)
		{
			string temp = t->name[0].c_str();
			semanticError(t->lineno, temp + "���Ǳ���");
		}
}

//д������������
void SemanticAnalysis::writestatement(TreeNode* t)
{
	TypeIR* Etp = Expr(t->child[0], NULL);

	if (Etp->kind == boolTy)
		semanticError(t->lineno, "���ʽ���Ϸ�");
}

void SemanticAnalysis::returnstatement(TreeNode* t)
{
	return;
}




