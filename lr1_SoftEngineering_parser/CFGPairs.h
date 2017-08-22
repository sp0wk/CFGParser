#ifndef CFGPAIRS
#define CFGPAIRS

#include <string>
#include <queue>
#include <list>
#include <stack>

using std::string;
using StringPair = std::pair<string, string>;
using PairsList = std::list<StringPair>; 


// struct for keeping loop's breaks and continues indices
struct LoopLabels 
{
	std::stack<string> _continues;
	std::stack<string> _breaks;
};

// struct for keeping switch's and "break" indices 
struct SwitchLabels
{
	std::string _switchIndex;
	bool _defaultFlag;
	std::stack<string> _breaks;
};


// Helping class of the Parser for index and pairs creation
class CFGpairs
{
private:
	unsigned int curInd;					// main index for statements
	string curIndStr;						// string representing of current index
	PairsList pairs;						// list with index pairs
	std::stack<LoopLabels*> loopstack;		// stack of loop-jump indices for nested loops
	std::stack<SwitchLabels*> switchstack;	// stack of switch and its breaks indices
	PairsList labels;						// list of named labels and theirs indices
	PairsList gotos;						// list of pending gotos label name and gotos indices
	bool jumpFlag;							// preceeding statement was break, continue, goto or return flag

public:
	//ctor and dtor
	CFGpairs() : curInd(1), curIndStr("1"), jumpFlag(false) {};
	~CFGpairs() {};

	//basic methods
	inline string getIndex() const { return curIndStr; }					// get current index number as string
	inline string newIndex() { return curIndStr=std::to_string(++curInd); }	// get incremented index as string
	inline bool getJumpFlag() const { return this->jumpFlag; }				// get jump flag
	inline void resetJumpFlag() { this->jumpFlag = false; }					// set jump flag as false
	string getFlowIndex();								// get current index (with correction for possible preceeding jump)
	void addStartIndex();								// add starting pair to pairs list
	void addExitIndex();								// add exiting pair to pairs list
	void addPair(string& f, string& s);					// add "first, second" pair to list
	void newPair();										// add pair of current and incremented index to list
	
	//// JUMP statement methods
	void addReturn();									// set flag and add pair to "EXIT"
	void addContinue();									// set flag and add current index to "continues" of last loop in stack
	void addBreak();									// set flag and add current index to "breaks" of last loop/switch in stack

	//// GOTO and LABEL methods
	void addLabel(string& lblname);						// add label name to list and link with pending gotos
	void addGoto(string& lblname);						// link goto index and existing label index or add to pending gotos
	void linkPendingGoto(string& lblname);				// link all pending goto indices with current label index
	
	//// LOOPSTACK methods
	void pushLoopToStack();								// add new loop to stack
	void popLoopFromStack();							// remove top loop from stack
	void makeLoopContinuePairs(string& pair_ind);		// link all "continue" indices with top loop in stack
	void makeLoopBreakPairs(string& pair_ind);			// link all "breaks" indices with top loop in stack

	//// SWITCHSTACK methods
	void pushSwitchToStack();							// add new switch to stack
	void popSwitchFromStack();							// remove top switch from stack
	void addCase();										// link case label index with switch
	void addDefault();									// link default label index with switch and set flag
	bool isDefaultPresent();							// get flag for default label in current switch
	void makeSwitchBreakPairs();						// link all switch's "breaks" with index after switch

	//MAIN method	
	string getAllPairs() const;							//return all pairs as one string
};

#endif