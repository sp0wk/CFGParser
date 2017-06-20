// DLL CFG Parser Header

#ifdef _BUILD_DLL_
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

// output struct with code and pairs
typedef struct {
	char* code;
	char* pairs;
} CodeAndPairs;


extern "C" DLL_API void __cdecl ParseCodeForCFG(CodeAndPairs* cpstruct, const char* code_in);
