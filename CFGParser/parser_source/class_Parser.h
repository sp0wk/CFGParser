#ifndef PARSER
#define PARSER

#include <string>
#include "class_GrammarCompiler.h"
#include "CFGPairs.h"

using std::string;


class Parser
{
private:
	GrammarCompiler* gc;			//pointer to grammar rules object
	CFGpairs cfgpairs;				//helping class for index and pairs creation
	string code_in;					//input code
	string code_out;				//output code with statement indices
	
	string makeIndent(int tabs);	//helping method to make indents for statements

	// PARSING METHODS
	void CompStat(string& stat_in, int tabs);			//parse compound statement
	void DeclStat(string& stat_in, int tabs);			//parse declaration statement
	void DowhileStat(string& stat_in, int tabs);		//parse do-while statement
	void ExprStat(string& stat_in, int tabs);			//parse expression statement
	void ForStat(string& stat_in, int tabs);			//parse "for" statement
	void IfStat(string& stat_in, int tabs);				//parse "if" statement
	void JumpStat(string& stat_in, int tabs);			//parse jump statement
	void LblStat(string& stat_in, int tabs);			//parse label statement
	void Stat(string& stat_in, int tabs);				//parse general statement
	void SwitchStat(string& stat_in, int tabs);			//parse switch statement
	void WhileStat(string& stat_in, int tabs);			//parse while statement
	void FuncDef(string& fdef_in);						//parse function

public:
	//ctor and dtor
	Parser(GrammarCompiler* gc, string& cppcode_in) : gc(gc), code_in(cppcode_in) {};
	~Parser() {};

	//DLL interface methods
	string getCode();		//get output code with indices
	string getPairs();		//get graph pairs as one string

	void ParseCode();		//Main method - Parse source code
};

#endif