#include <windows.h>

#include <iostream>
#include <fstream>
#include <string>
#include "parser_source/class_Parser.h"
// for CoTaskMemFree
//#include <objbase.h>
//#pragma comment(lib,"ole32.lib")


typedef struct {
	char* code;
	char* pairs;
} CodeAndPairs;


LPSTR dll_name = "CFGParserDLL.dll";
typedef void (__cdecl *TReturnFunc)(CodeAndPairs*, const char*);


int main(int argc, char* argv[]) 
{
	if (argc == 1)	argv[1] = "ccode.txt";

	std::ifstream file_in(argv[1]);
	std::string code = "";

	if (file_in.good()) {
		std::cout << "File opened\n\n";
		char c;
		while(file_in.get(c)) {
			code += c;
		}
	}

	std::string dllcode = code;
	//CodeAndPairs res;
	GrammarCompiler gc;
	Parser p(&gc, code);
	p.ParseCode();

	/*HINSTANCE DLL_load = LoadLibrary(dll_name);
	if (DLL_load != NULL) {
		std::cout << "DLL LOADED" << "\n\n";
		// GetProcAddress returns a pointer to the loaded method
		TReturnFunc CPFunc = (TReturnFunc)GetProcAddress(DLL_load, "ParseCodeForCFG");

		// Set changed structure with code and pairs
		CPFunc(&res, dllcode.c_str());
*/
		std::cout << "OUTPUT CODE:\n\n" << /*res.code*/p.getCode();
		system("pause");

		std::cout << "\nCFG PAIRS:\n\n" << /*res.pairs*/p.getPairs();
		system("pause");
/*
		//free memory
		CoTaskMemFree(res.code);
		CoTaskMemFree(res.pairs);
	
		// unload DLL
		FreeLibrary(DLL_load);
	}
*/

	return 0;
}