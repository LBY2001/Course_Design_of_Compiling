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

//标识符的类型
typedef  enum { typeKind, varKind, procKind }IdKind;

//变量的类别。dir表直接变量(值参)，indir表示间接变量(变参)
typedef  enum { dir, indir }AccessKind;

//形参表的结构定义
typedef struct  paramTable
{
	struct symbtable* entry;		//指向该形参所在符号表中的地址入口
	struct paramTable* next;		//指向下一个形参
}ParamTable;

//标识符结构
typedef struct
{
	struct typeIR* idtype;			//指向标识符的类型内部表示
	IdKind kind;					//标识符的类型
	union
	{
		struct
		{
			AccessKind access;		//判断是变参还是值参
			int level;
			int off;
			bool isParam;			//判断是参数还是普通变量

		}VarAttr;					//变量标识符的属性
		struct
		{
			int level;				/*该过程的层数*/
			ParamTable* param;		/*参数表*/
			int mOff;				/*过程活动记录的大小*/
			int nOff;  				/*sp到display表的偏移量*/
			int procEntry;			/*过程的入口地址*/
			int codeEntry;			/*过程入口标号,用于中间代码生成*/
		}ProcAttr;					/*过程名标识符的属性*/
	}More;							/*标识符的不同类型有不同的属性*/
}AttributeIR;

//符号表的结构定义
typedef struct  symbtable
{
	string idName;
	AttributeIR  attrIR;
	struct symbtable* next;

}SymbTable;

/* 类型内部表示 */
//类型的枚举定义
typedef  enum { intTy, charTy, arrayTy, recordTy, boolTy }TypeKind;

//域类型单元结构定义
typedef struct fieldchain
{
	string id;					//变量名
	int    off;                 //所在记录中的偏移
	struct typeIR* UnitType;	//域中成员的类型
	struct fieldchain* Next;
}fieldChain;


//类型的内部结构定义
typedef struct typeIR
{
	int	size;   /*类型所占空间大小*/
	TypeKind kind;
	union
	{
		struct
		{
			struct typeIR* indexTy;	//指向数组下标类型
			struct typeIR* elemTy;	//指向数组元素类型
			int low;     //下界
			int up;      //上界
		}ArrayAttr;
		fieldChain* body;  //指向域成员的链表
	} More;
}TypeIR;

class SemanticAnalysis
{
public:
	//其他函数
	void initial();				//全局变量初始化
	void fileClose();			//文件关闭
	void semanticError(int line, string errorMessage);
	SymbTable* NewTable(void);	//新建空符号表
	TypeIR* NewTy(TypeKind kind);//新建类型内部表示
	fieldChain* NewBody();		//新建域
	ParamTable* NewParam();		//新建形参链表 

	//符号表实现
	void CreatTable(void);		//创建符号表
	void DestroyTable();		//撤销符号表
	bool Enter(string id, AttributeIR* AttribP, SymbTable** Entry);
								//登记标识符和属性到符号表
	bool FindEntry(string id, SymbTable** entry);
	bool FindField(string Id, fieldChain* head, fieldChain** Entry);
	void PrintSymbTable();		//打印符号表


	//检查语义信息
	void analyze(TreeNode* currentP);	//语义分析主函数
	TypeIR* TypeProcess(TreeNode* t, DecKind deckind);
										//类型分析处理
	TypeIR* nameType(TreeNode* t);		//自定义类型内部结构分析
	TypeIR* arrayType(TreeNode* t);		//数组类型
	TypeIR* recordType(TreeNode* t);	//记录类型
	void TypeDecPart(TreeNode* t);		//类型声明部分分析处理函数
	void varDecList(TreeNode* t);		//变量声明部分分析处理函数
	void procDecPart(TreeNode* t);		//过程声明部分分析处理函数
	SymbTable* HeadProcess(TreeNode* t);//过程声明头分析函数
	ParamTable* ParaDecList(TreeNode* t);//形参分析处理函数
	void Body(TreeNode* t);				//执行体部分分析处理函数
	void statement(TreeNode* t);		//语句序列分析处理函数
	TypeIR* Expr(TreeNode* t, AccessKind* Ekind);
										//表达式分析处理函数
	TypeIR* arrayVar(TreeNode* t);		//数组变量的处理分析函数
	TypeIR* recordVar(TreeNode* t);		//记录变量中域变量的分析处理函数
	void assignstatement(TreeNode* t);	//赋值语句分析函数
	void callstatement(TreeNode* t);	//过程调用语句分析处理函数
	void ifstatment(TreeNode* t);		//条件语句分析处理函数
	void whilestatement(TreeNode* t);	//循环语句分析处理函数
	void readstatement(TreeNode* t);	//读语句分析处理函数
	void writestatement(TreeNode* t);	//写语句分析处理函数
	void returnstatement(TreeNode* t);	//返回语句分析处理函数
};