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


SymbTable* scope[1000];			//scope栈
int scopeLevel;					//scope层数
int Off;						//同层的变量偏移量
ofstream errorFile;				//输出错误信息文件
ofstream tableFile;				//输出符号表文件
bool hasError;
TypeIR* intPtr;					//指向整数类型的内部表示
TypeIR* charPtr;				//指向字符类型的内部表示
TypeIR* boolPtr;				//指向布尔类型的内部表示

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

//输出语义错误
void SemanticAnalysis::semanticError(int line, string errorMessage)
{
	errorFile << "语义错误, line " << line << " : " << errorMessage << endl;
	hasError = true;
}

//新建空符号表
SymbTable* SemanticAnalysis::NewTable(void)
{
	SymbTable* table = new SymbTable;
	if (table == NULL)
	{
		errorFile << "内存溢出 " << endl;
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
		errorFile << "内存溢出 " << endl;
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
		errorFile << "内存溢出 " << endl;
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
		errorFile << "内存溢出 " << endl;
		hasError = true;
	}
	else
	{
		Ptr->entry = NULL;
		Ptr->next = NULL;
	}
	return Ptr;
}

//建立空符号表table
void SemanticAnalysis::CreatTable(void)
{
	Off = 7;						//教材P161定为常数7
	scopeLevel++;					//层数加1
	scope[scopeLevel] = NULL;		//新scope栈
}

void SemanticAnalysis::DestroyTable()
{
	scopeLevel--;
}

//登记
bool SemanticAnalysis::Enter(string id, AttributeIR* attribP, SymbTable** entry)
{
	bool present = false;
	SymbTable* pNow = scope[scopeLevel];
	SymbTable* pLast = scope[scopeLevel];

	//空表不存在重复定义，直接建立符号表项
	if (scope[scopeLevel] == NULL)
	{
		pNow = NewTable();
		scope[scopeLevel] = pNow;
	}
	
	//否则判断重复定义错误
	//查找单层
	else
	{
		//在该层符号表内检查重复定义错误,结果录入present
		while ((pNow != NULL) && (present != true))
		{
			pLast = pNow;
			if (id == pNow->idName)
			{
				errorFile << "标识符重复声明 " << endl;
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

	//？标识符重复声明错，直接返回
	if (present == true)
		return true;

	//将标识符名和属性登记到表中
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

//查找全部符号表
bool SemanticAnalysis::FindEntry(string id, SymbTable** entry)
{
	bool present = false;
	int level = scopeLevel;		//临时记录层数

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
		//查找下一个局部化区域
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

//打印符号表
void SemanticAnalysis::PrintSymbTable()
{
	int level = 0;
	while (scope[level] != NULL)
	{
		SymbTable* t = scope[level];
		tableFile << "==========第" << level << "层符号表==========" << endl;
		while (t != NULL)
		{
			//标识符名
			tableFile << t->idName << ":  ";
			AttributeIR* Attrib = &(t->attrIR);

			//标识符类型
			//？非过程标识符
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

			//标识符类别，及相关信息
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
	/*创建符号表*/
	CreatTable();

	//循环处理主程序的声明部分
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
				semanticError(p->lineno, "声明类型异常");
				break;
		}
		p = p->sibling;
	}

	//处理主程序体
	currentP = currentP->child[2];
	if (currentP->nodekind == StmLK)
		Body(currentP);

	//撤销符号表
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

	//符号表中找到类型名
	if (FindEntry(t->attr.type_name, &entry))
	{
		//检查该标识符是否为类型标识符
		if (entry->attrIR.kind != typeKind)
			semanticError(t->lineno, "非类型标识符错");
		else
			Ptr = entry->attrIR.idtype;
	}
	//未找到类型名
	else
	{
		semanticError(t->lineno, "无声明错误");
	}
	return Ptr;
}

TypeIR* SemanticAnalysis::arrayType(TreeNode* t)
{
	TypeIR* present = NULL;
	if (t->attr.ArrayAttr.low > t->attr.ArrayAttr.up)
	{
		semanticError(t->lineno, "数组越界");
		hasError = true;
	}
	else
	{
		//present->kind也在这赋值
		present = NewTy(arrayTy);
		present->size = (t->attr.ArrayAttr.up - t->attr.ArrayAttr.low + 1) * present->size;
		//？数组下表类型，应该是只存在整形
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

	fieldChain* pLast = NULL;	//若存在多个id，此为上一个节点指针
	fieldChain* pNow = NULL;	//若存在多个id，此为最新指针
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

			//pLast!=pNow，那么将指针后移
			if (pNow != pLast)
			{
				//计算新申请的单元off
				pNow->off = pLast->off + pLast->UnitType->size;
				pLast->Next = pNow;
				pLast = pNow;
			}
		}
		t = t->sibling;
	}
	return Ptr;
}

//检查本层类型声明中是否有重复定义错误
void SemanticAnalysis::TypeDecPart(TreeNode* t)
{
	AttributeIR attrI;
	SymbTable* entry = NULL;
	//Kind为变量类型
	attrI.kind = typeKind;
	while (t != NULL)
	{
		if (Enter(t->name[0], &attrI, &entry) != false)
		{
			string temp = t->name[0].c_str();
			semanticError(t->lineno, temp + "重复声明报错");
			entry = NULL;
		}
		else
			entry->attrIR.idtype = TypeProcess(t, t->kind.dec);
		t = t->sibling;
	}
}

//检查本层变量声明中是否有重复定义错误
void SemanticAnalysis::varDecList(TreeNode* t)
{
	AttributeIR attrIr;
	SymbTable* entry = NULL;
	while (t != NULL)
	{
		//Kind为变量类型
		attrIr.kind = varKind;
		//循环处理同一个节点的id，调用类型处理函数
		for (int i = 0; i < t->idnum; i++)
		{
			attrIr.idtype = TypeProcess(t, t->kind.dec);

			//判断是否为直接变量
			if (t->attr.ProcAttr.paramt == valparamType)
			{
				//直接变量
				attrIr.More.VarAttr.access = dir;
				attrIr.More.VarAttr.level = scopeLevel;
				attrIr.More.VarAttr.off = Off;
				Off = Off + attrIr.idtype->size;
			}//偏移加变量类型的size

			else
			{
				//间接变量(变参)
				attrIr.More.VarAttr.access = indir;
				attrIr.More.VarAttr.level = scopeLevel;
				attrIr.More.VarAttr.off = Off;
				Off = Off + 1;
			}//P168 间接变量，偏移加1
			

			//登记该变量的属性及名字,并判断是否被重复定义
			if (Enter(t->name[i], &attrIr, &entry) != false)
			{
				string temp = t->name[i].c_str();
				semanticError(t->lineno, temp + "重复声明报错");
				entry = NULL;
			}
			else
				t->table[i] = entry;
				//记录类型名
		}
		t = t->sibling;
	}

}

//在当前层符号表添写过程符号表属性，在新层符号表填写形参标识符属性
void SemanticAnalysis::procDecPart(TreeNode* t)
{
	SymbTable* entry = HeadProcess(t);
	t = t->child[1];
	while (t != NULL)
	{
		//处理声明部分
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
				semanticError(t->lineno, "声明类型异常");
				break;
		}
		t = t->sibling;
	}

	//结束当前scope
	DestroyTable();
}

//过程声明头分析函数
SymbTable* SemanticAnalysis::HeadProcess(TreeNode* t)
{
	AttributeIR attrIr;
	SymbTable* entry = NULL;

	//填写属性
	attrIr.idtype = NULL;
	attrIr.kind = procKind;
	attrIr.More.ProcAttr.level = scopeLevel + 1;

	//登记表项喽
	bool temp = Enter(t->name[0], &attrIr, &entry);
	t->table[0] = entry;

	//调用形参处理函数
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

		//进入新局部化区
		CreatTable();

		//调用函数varDecPart
		varDecList(p);

		//？需要循环吗
		//构造形参符号表
		SymbTable* temp = scope[scopeLevel];
		ParamTable* Ptemp;//符号表
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
			semanticError(t->lineno, "无效语句类型");
			break;
	}
}

//检查运算分量的类型相容性，求表达式的类型。Ekind用来表示实参是变参还是值参
TypeIR* SemanticAnalysis::Expr(TreeNode* t, AccessKind* Ekind)
{
	//指向表达式类型内部表示的指针
	TypeIR* Eptr = NULL;
	SymbTable* entry;
	switch (t->kind.exp)
	{
		//常量处理
		case ConstK:
			Eptr = TypeProcess(t, IntegerK);
			Eptr->kind = intTy;
			if (Ekind != NULL)
				(*Ekind) = dir;   //直接变量
			break;

		//变量处理
		case VariK:
			//寻常变量
			if (t->child[0] == NULL)
			{
				bool tempb = FindEntry(t->name[0], &entry);
				t->table[0] = entry;
				if (tempb == true)
				{   
					//判断是否为间接变量
					if (entry->attrIR.kind == varKind)
					{
						Eptr = entry->attrIR.idtype;
						if (Ekind != NULL)
							*Ekind = indir;  //间接变量
					}
					else
					{
						string temp = t->name[0].c_str();
						semanticError(t->lineno, temp + "不是变量");
						Eptr = NULL;
					}
				}
				else
				{
					string temp = t->name[0].c_str();
					semanticError(t->lineno, temp + "声明缺失");
				}

			}
			//数组变量或域变量
			else
			{
				//数组变量 Var = Var0[E]
				if (t->attr.ExpAttr.varkind == ArrayMembV)
					Eptr = arrayVar(t);
				//域变量 Var = Var0.id
				else if (t->attr.ExpAttr.varkind == FieldMembV)
					Eptr = recordVar(t);
				else
				{
					string temp = t->name[0].c_str();
					semanticError(t->lineno, temp + "不是数组或域变量");
				}
			}
			break;

		//操作符
		case OpK:
			TypeIR* op1 = NULL;	//操作数1
			TypeIR* op2 = NULL;	//操作数2
			op1 = Expr(t->child[0], NULL);
			op2 = Expr(t->child[1], NULL);

			if (op1 == NULL || op2 == NULL)
			{
				semanticError(t->lineno, "不是数组或域变量");
				return NULL;
			}

			if (op1 == op2)
			{
				switch (t->attr.ExpAttr.op)
				{
					case LT:
					case EQ:
						Eptr = boolPtr;
						break;  /*条件表达式*/
					case PLUS:
					case MINUS:
					case TIMES:
					case OVER:
						Eptr = intPtr;
						break;  /*算数表达式*/
				}
				if (Ekind != NULL)
					(*Ekind) = dir; /*直接变量*/
			}
			else
				semanticError(t->lineno, "操作数类型不兼容");
			break;
	}
	return Eptr;
}

//数组变量的处理分析函数
TypeIR* SemanticAnalysis::arrayVar(TreeNode* t)
{
	TypeIR* Eptr = NULL;
	SymbTable* entry = NULL;
	if (FindEntry(t->name[0], &entry))
	{
		string temp = t->name[0].c_str();
		semanticError(t->lineno, temp + "未找到此标识符");
		entry = NULL;
	}
	else
	{
		if (entry->attrIR.kind != varKind)
		{
			string temp = t->name[0].c_str();
			semanticError(t->lineno, temp + "不是变量");
		}
		else
		{
			if (entry->attrIR.idtype != NULL)
			{
				string temp = t->name[0].c_str();
				semanticError(t->lineno, temp + "不是数组变量");
			}
			else
			{
				//数组下标类型
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
					semanticError(t->lineno, temp + "与下标类型不相符");
				}
			}
		}
	}
	return Eptr;
}

//记录变量中域变量的分析处理函数
TypeIR* SemanticAnalysis::recordVar(TreeNode* t)
{
	TypeIR* Eptr = NULL;
	SymbTable* entry = NULL;
	if (FindEntry(t->name[0], &entry) == false)
	{
		string temp = t->name[0].c_str();
		semanticError(t->lineno, temp + "未找到此标识符");
	}
	else
	{
		if (entry->attrIR.kind != varKind)
		{
			string temp = t->name[0].c_str();
			semanticError(t->lineno, temp + "不是变量");
		}
		else
		{
			if (entry->attrIR.idtype->kind != recordTy)
			{
				string temp = t->name[0].c_str();
				semanticError(t->lineno, temp + "不是变量");
			}
			//检查合法域名,看id名是否在域里
			else
			{
				fieldChain* tempF = NULL;
				tempF = entry->attrIR.idtype->More.body;
				while (tempF != NULL)
				{
					//不相等测下一个
					if (t->child[0]->name[0] != tempF->id)
						tempF = tempF->Next;
					else
						Eptr = tempF->UnitType;
				}

				
				if (tempF == NULL)
				{
					string temp = t->name[0].c_str();
					semanticError(t->lineno, temp + "不是合法域名");
				}
				else
				{
					//?数组变量
					if (t->child[0]->child != NULL)
						Eptr = arrayVar(t->child[0]);
				}
			}
		}
	}
	return Eptr;
}

//赋值语句分析
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
			semanticError(t->lineno, temp + "未找到此标识符");
			entry = NULL;
		}
		else
		{
			if (entry->attrIR.kind != varKind)
			{
				string temp = t->name[0].c_str();
				semanticError(t->lineno, temp + "不是变量");
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
		//? child1->attr.ExpAttr.varkind==FieldMembV 是否判断
		else
			Eptr = recordVar(child1);
	}

	//检查赋值号两侧是否等价
	if (Eptr != NULL)
		if (Expr(child2, NULL) != Eptr)
			semanticError(t->lineno, "等号两侧不等价");
}

//过程语句分析
void SemanticAnalysis::callstatement(TreeNode* t)
{
	SymbTable* entry = NULL;
	bool temp = FindEntry(t->child[0]->name[0], &entry);
	t->child[0]->table[0] = entry;

	if (temp == false)
	{
		string temp = t->name[0].c_str();
		semanticError(t->lineno, temp + "未找到此函数");
	}
	else
	{
		if (entry->attrIR.kind != procKind)
		{
			string temp = t->name[0].c_str();
			semanticError(t->lineno, temp + "不是函数名");
		}
		//判断形参实参对应
		else
		{
			//参数表指针
			ParamTable* tempTable = entry->attrIR.More.ProcAttr.param;
			//实参指针
			TreeNode* pNode = t->child[1];

			while (tempTable != NULL && pNode != NULL)
			{
				AccessKind tempA;
				TypeIR* tempT = Expr(pNode, &tempA);

				if ((tempTable->entry->attrIR.More.VarAttr.access == indir) && (tempA == dir))
					semanticError(t->lineno, "值参类型不得使用变参");
				else
					if ((tempTable->entry->attrIR.idtype) != tempT)
						semanticError(t->lineno, "参数类型不匹配");
				
				pNode = pNode->sibling;
				tempTable = tempTable->next;
			}

			//参数数量不匹配
			if (tempTable != NULL || pNode != NULL)
				semanticError(t->lineno, "参数个数不匹配");
		}

	}
}

//条件语句分析处理函数
void SemanticAnalysis::ifstatment(TreeNode* t)
{
	TypeIR* Etp = Expr(t->child[0], NULL);

	//条件表达式是bool型
	if (Etp->kind != boolTy)
		semanticError(t->lineno, "表达式非bool型");
	else
	{
		//then语句
		TreeNode* p = t->child[1];
		while (p != NULL)
		{
			statement(p);
			p = p->sibling;
		}

		//else语句
		p = t->child[2];
		while (p != NULL)
		{
			statement(p);
			p = p->sibling;
		}
	}
}

//循环语句分析处理函数
void SemanticAnalysis::whilestatement(TreeNode* t)
{
	TypeIR* Etp = Expr(t->child[0], NULL);

	//条件表达式是bool型
	if (Etp->kind != boolTy)
		semanticError(t->lineno, "表达式非bool型");
	else
	{
		//语句序列
		TreeNode* p = t->child[1];
		while (p != NULL)
		{
			statement(p);
			p = p->sibling;
		}
	}
}

//读语句分析处理函数
void SemanticAnalysis::readstatement(TreeNode* t)
{
	SymbTable* entry = NULL;
	if (FindEntry(t->name[0], &entry) == false)
	{
		string temp = t->name[0].c_str();
		semanticError(t->lineno, temp + "未找到此标识符");
	}
	else
		if (entry->attrIR.kind != varKind)
		{
			string temp = t->name[0].c_str();
			semanticError(t->lineno, temp + "不是变量");
		}
}

//写语句分析处理函数
void SemanticAnalysis::writestatement(TreeNode* t)
{
	TypeIR* Etp = Expr(t->child[0], NULL);

	if (Etp->kind == boolTy)
		semanticError(t->lineno, "表达式不合法");
}

void SemanticAnalysis::returnstatement(TreeNode* t)
{
	return;
}




