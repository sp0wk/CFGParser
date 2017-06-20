#define _BUILD_DLL_
#include "DLL_CFGparser.h"
#include <stdlib.h>
#include "parser_source\class_GrammarCompiler.h"
#include "parser_source\class_Parser.h"
#include <string>
#include <objbase.h>
#pragma comment(lib,"ole32.lib")


DLL_API void __cdecl ParseCodeForCFG(CodeAndPairs* cpstruct, const char* code_in)
{
	static GrammarCompiler gc;
	std::string codestring = string(code_in);

	// code parsing
	Parser p(&gc, codestring);
	p.ParseCode();

	std::string strcode = p.getCode();
	std::string strpairs = p.getPairs();
	size_t oc_size = sizeof(char) * (strcode.size() + 1);
	size_t op_size = sizeof(char) * (strpairs.size() + 1);

	//allocate memory using COM CoTaskMemAlloc for C# CLR to being able to correctly free it
	cpstruct->code = (char*) CoTaskMemAlloc(oc_size);
	cpstruct->pairs = (char*) CoTaskMemAlloc(op_size);
	
	//copy data
	strcpy(cpstruct->code, strcode.c_str());
	strcpy(cpstruct->pairs, strpairs.c_str());
}