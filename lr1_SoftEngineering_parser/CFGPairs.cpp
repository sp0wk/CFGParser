#include "CFGPairs.h"
#include <algorithm>



//ctor and dtor
CFGpairs::CFGpairs() : curInd(1), curIndStr("1"), jumpFlag(false) {};
CFGpairs::~CFGpairs() {};


////basic methods

// get current index (with correction for preceeding jump)
std::string CFGpairs::getFlowIndex() 
{
	// if previous statement was jump statement
	if (jumpFlag) {
		jumpFlag = false;
		return newIndex();
	}
	
	return curIndStr;
}

// add starting pair to pairs list
void CFGpairs::addStartIndex() 
{ 
	pairs.push_back({ "START", "1" }); 
}

// add exiting pair to pairs list
void CFGpairs::addExitIndex() 
{ 
	if (!jumpFlag) pairs.push_back({ curIndStr, "EXIT" }); 
}

// add "first, second" pair to list
void CFGpairs::addPair(std::string& first, std::string& second) 
{ 
	if (first != second) pairs.push_back({ first, second }); 
}

// add pair of current and incremented index to list
void CFGpairs::newPair() 
{ 
	std::string oldindex = curIndStr;
	pairs.push_back({ oldindex, newIndex() }); 
}


//// JUMP statement methods

// set flag and add pair to "EXIT"
void CFGpairs::addReturn() 
{
	jumpFlag = true;
	addPair(curIndStr, std::string("EXIT"));
}

// set flag and add current index to "continues" of last loop in stack
void CFGpairs::addContinue() 
{
	jumpFlag = true;

	if (!loopstack.empty()) {
		(loopstack.top())->_continues.push(curIndStr);
	}
}

// set flag and add current index to "breaks" of last loop/switch in stack
void CFGpairs::addBreak() 
{
	jumpFlag = true;
	
	// if "break" in switch scope
	if (!switchstack.empty()) {
		(switchstack.top())->_breaks.push(curIndStr);
	}
	else { //in loop scope
		if (!loopstack.empty()) {
			(loopstack.top())->_breaks.push(curIndStr);
		}
	}
}


//// GOTO and LABEL methods

// add label name to list and link with pending gotos
void CFGpairs::addLabel(std::string& lblname) 
{
	// add to label list
	labels.push_back({ lblname, curIndStr });
	//link with pending goto's
	linkPendingGoto(lblname);
}

// link goto index and existing label index or add to pending gotos
void CFGpairs::addGoto(std::string& lblname) 
{
	jumpFlag = true;
	
	//find label name in list
	PairsList::iterator iter = std::find_if(labels.begin(), labels.end(), [lblname] (StringPair & m) { return  lblname == m.first; });

	//if label found
	if (iter != labels.end()) {
		this->addPair(curIndStr, (*iter).second); // make pair with "goto" index and label index
	}
	else { //add pending goto's label and index for future linking with label
		this->gotos.push_back({ lblname, curIndStr });
	}
}

// link all pending goto indices with current label index
void CFGpairs::linkPendingGoto(std::string& lblname) 
{
	if (!gotos.empty()) {
		//for each label name in pending goto list
		for (PairsList::iterator iter = gotos.begin(); iter != gotos.end(); ++iter) {
			if ((*iter).first == lblname) {
				addPair((*iter).second, curIndStr);		//add pair for goto index and label index
				iter = gotos.erase(iter);				//remove pending goto from list
				
				if (iter == gotos.end()) break;
			}
		}
	}		
}


//// LOOPSTACK methods

// add new loop to stack
void CFGpairs::pushLoopToStack() 
{
	loopstack.push(new LoopLabels());
}

// remove top loop from stack
void CFGpairs::popLoopFromStack() 
{
	if (!loopstack.empty()) {
		delete loopstack.top();
		loopstack.pop();
	}
}

// link all "continue" indices with top loop in stack
void CFGpairs::makeLoopContinuePairs(std::string& pair_ind) 
{
	LoopLabels* currentloop = loopstack.top();
	while ( !currentloop->_continues.empty() ) {
		this->addPair(currentloop->_continues.top(), pair_ind);
		currentloop->_continues.pop();
	}
}

// link all "breaks" indices with top loop in stack
void CFGpairs::makeLoopBreakPairs(std::string& pair_ind) 
{
	LoopLabels* currentloop = loopstack.top();
	while ( !currentloop->_breaks.empty() ) {
		this->addPair(currentloop->_breaks.top(), pair_ind);
		currentloop->_breaks.pop();
	}
}


//// SWITCHSTACK methods

// add new switch to stack
void CFGpairs::pushSwitchToStack() 
{
	switchstack.push( new SwitchLabels({ this->getIndex(), false }) );
	this->jumpFlag = true;	//switch is kind of multi-goto (no next flow)
}

// remove top switch from stack
void CFGpairs::popSwitchFromStack() 
{
	if (!switchstack.empty()) {
		delete switchstack.top();
		switchstack.pop();
	}
}

// link case label index with switch
void CFGpairs::addCase()
{
	if (!switchstack.empty()) {
		this->addPair(switchstack.top()->_switchIndex, this->getIndex());
	}
}

// link default label index with switch and set flag
void CFGpairs::addDefault()
{
	if (!switchstack.empty()) {
		this->addPair(switchstack.top()->_switchIndex, this->getIndex());
		switchstack.top()->_defaultFlag = true;
	}
}
	
// flag for default label in current switch
bool CFGpairs::isDefaultPresent()
{
	return switchstack.empty() ? false : switchstack.top()->_defaultFlag;
}

// link all "breaks" indices with top switch in stack
void CFGpairs::makeSwitchBreakPairs() 
{
	SwitchLabels* currentswitch = switchstack.top();
	while ( !currentswitch->_breaks.empty() ) {
		this->addPair(currentswitch->_breaks.top(), this->getIndex());
		currentswitch->_breaks.pop();
	}
}


//MAIN method
//return all pairs as one string
std::string CFGpairs::getAllPairs() const
{
	std::string result = "";

	for (PairsList::const_iterator iter = pairs.begin(); iter != pairs.end(); iter++) {
		result += (*iter).first + " -> " + (*iter).second + ";\n";
	}

	return result;
}