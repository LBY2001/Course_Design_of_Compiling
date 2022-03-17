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
 ******************   语法分析树   ********************
 ******************************************************/

 /*语法树根节点ProK,程序头结点PheadK，声明类型节点DecK,
   标志子结点都是类型声明的结点TypeK,标志子结点都是变量声明的结点VarK,
   函数声明结点FuncDecK,语句序列节点StmLK,语句声明结点StmtK,
   表达式结点ExpK*/
typedef enum { ProK, PheadK, DecK, TypeK, VarK, ProcDecK, StmLK, StmtK, ExpK }
NodeKind;

/*声明类型Deckind 类型的枚举定义：
  数组类型ArrayK,字符类型CharK,
  整数类型IntegerK,记录类型RecordK,
  以类型标识符作为类型的IdK*/
typedef enum { ArrayK, CharK, IntegerK, RecordK, IdK }  DecKind;

/* 语句类型StmtKind类型的枚举定义:			*
 * 判断类型IfK,循环类型WhileK				*
 * 赋值类型AssignK,读类型ReadK              *
 * 写类型WriteK，函数调用类型CallK          */
typedef enum { IfK, WhileK, AssignK, ReadK, WriteK, CallK, ReturnK } StmtKind;


/* 表达式类型ExpKind类型的枚举定义:         *
 * 操作类型OpK,常数类型ConstK,变量类型VarK  */
typedef enum { OpK, ConstK, VariK } ExpKind;


/* 变量类型VarKind类型的枚举定义:           *
 * 标识符IdV,数组成员ArrayMembV,域成员FieldMembV*/
typedef enum { IdV, ArrayMembV, FieldMembV } VarKind;


/* 类型检查ExpType类型的枚举定义:           *
 * 空Void,整数类型Integer,字符类型Char      */
typedef enum { Void, Integer, Boolean } ExpType;

/* 参数类型ParamType类型的枚举定义：        *
 * 值参valparamType,变参varparamType        */
typedef enum { valparamType, varparamType } ParamType;

/*提前声明符号表结构*/
struct symbtable;

/********** 语法树节点treeNode类型 *********/
typedef struct treeNode

{
	struct treeNode* child[3];		/* 子节点指针	*/
	struct treeNode* sibling;		/* 兄弟节点指针	*/
	int lineno;						/* 源代码行号	*/
	NodeKind nodekind;				/* 节点类型		*/
	union
	{
		DecKind  dec;
		StmtKind stmt;
		ExpKind  exp;
	} kind;                       /* 具体类型     */

	int idnum;                    /* 相同类型的变量个数 */

	string name[10];            /* 标识符的名称  */

	struct symbtable* table[10];  /* 与标志符对应的符号表地址，在语义分析阶段填入*/

	struct
	{
		struct
		{
			int low;              /* 数组下界     */
			int up;               /* 数组上界     */
			DecKind   childtype;  /* 数组的子类型 */
		}ArrayAttr;               /* 数组属性     */

		struct
		{
			ParamType  paramt;     /* 过程的参数类型 */
		}ProcAttr;                 /* 过程属性       */

		struct
		{
			LexType op;           /* 表达式的操作符*/
			int val;		      /* 表达式的值	   */
			VarKind  varkind;     /* 变量的类别    */
			ExpType type;         /* 用于类型检查  */
		}ExpAttr;	              /* 表达式属性    */

		string type_name;       /* 类型名是标识符 */

	} attr;                       /* 属性	        */
}TreeNode;

class RecursiveDescentParsing
{
public:
	void initial();							//全局变量初始化
	void ReadNextToken();					//读文件
	void syntaxError(string errorMessage);	//输出语法错误
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