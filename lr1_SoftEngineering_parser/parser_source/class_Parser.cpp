#include "class_Parser.h"



//make indent string
string Parser::makeIndent(int tabs) {
	string indent = "";
	for (int i = 0; i < tabs; i++)
		indent += "\t";

	return indent;
}

//get output code
string Parser::getCode()
{
	return this->code_out;
}

//get graph pairs
string Parser::getPairs() 
{
	return cfgpairs.getAllPairs();
}


///////  PARSING ///////////

//parse function
void Parser::FuncDef(string& fdef_in)
{
	smatch m;
	regex_match(fdef_in, m, gc->func_def);
	this->code_out += m[4].str();

	//parse function body
	cfgpairs.addStartIndex();
	CompStat(m[3].str(), 1);
	cfgpairs.addExitIndex();
	
	this->code_out += "\n\n";
}



// ============================================================================
// Statements
//=============================================================================

//parse compound statement
void Parser::CompStat(string& stat_in, int tabs)
{
	string statements = "";
	string indent = makeIndent(tabs-1);	//correct indent

	this->code_out += indent + "{\n";
	
	smatch m;
	regex_match(stat_in, m, gc->comp_stat);
	statements = m[1].str();

	//parse statements between braces
	sregex_iterator end;
	for (sregex_iterator iter(cbegin(statements), cend(statements), gc->stat_space); iter != end; iter++) {
		Stat((*iter)[1].str(), tabs);
		this->code_out += "\n";
	}

	this->code_out += indent + "}";
}


//parse declaration statement
void Parser::DeclStat(string& stat_in, int tabs)
{
	string indent = makeIndent(tabs-1);
	string index = cfgpairs.getFlowIndex();

	this->code_out += indent + "/* " + index + " */ " + stat_in;
}


//parse do-while statement
void Parser::DowhileStat(string& stat_in, int tabs)
{
	string indent = makeIndent(tabs-1);
	string index = cfgpairs.getFlowIndex();

	smatch m;
	regex_match(stat_in, m, gc->dowhile_stat);	

	this->code_out += indent + "/* " + index + " */ " + "do\n";
	
	//add loop to stack
	cfgpairs.pushLoopToStack();

	//add pair and create new index for do-block
	cfgpairs.newPair();
	
	//parse block
	Stat(m[1].str(), tabs+1);

	this->code_out += "\n" + indent + "/* " + cfgpairs.getIndex() + " */ " + "while (" + m[2].str() + ");";

	//make pairs for "continues"
	cfgpairs.makeLoopContinuePairs(cfgpairs.getIndex());

	if (!cfgpairs.getJumpFlag()) {
		//save last statement index
		string dw_last_stat = cfgpairs.getIndex();

		//create pair for last loop statement and beginning of loop
		cfgpairs.addPair(dw_last_stat, index);

		//make pairs for "breaks"
		cfgpairs.makeLoopBreakPairs(cfgpairs.newIndex());

		//create pair for last loop statement and statement after "while" 
		cfgpairs.addPair(dw_last_stat, cfgpairs.getIndex());			
	}
	else {
		//make pairs for "breaks"
		cfgpairs.makeLoopBreakPairs(cfgpairs.newIndex());
		// reset flag for last loop statement
		cfgpairs.resetJumpFlag();
	}

	this->code_out += "\n" + indent + "/* " + cfgpairs.getIndex() + " */";	//add empty block after do-while loop

	//pop loop from stack
	cfgpairs.popLoopFromStack();		
}


//parse expression statement
void Parser::ExprStat(string& stat_in, int tabs)
{
	string indent = makeIndent(tabs-1);
	string index = cfgpairs.getFlowIndex();

	this->code_out += indent + "/* " + index + " */ " + stat_in;
}


//parse "for" statement
void Parser::ForStat(string& stat_in, int tabs)
{
	string indent = makeIndent(tabs-1);
	string index = cfgpairs.getFlowIndex();
	string forstat = "";

	smatch m;
	regex_match(stat_in, m, gc->for_stat);
	forstat = m[4].str();
	

	this->code_out += indent + "/* " + index + " */ " + m[5].str() + "\n";		//add "for (...)" part to code

	//add loop to stack
	cfgpairs.pushLoopToStack();

	//add new pair and create new index for for-block
	cfgpairs.newPair();

	//parse statement part
	Stat(forstat, tabs+1);

	//make pairs for "continues"
	cfgpairs.makeLoopContinuePairs(index);

	//create pair for last loop statement and beginning of loop
	if (!cfgpairs.getJumpFlag()) cfgpairs.addPair(cfgpairs.getIndex(), index);
	else cfgpairs.resetJumpFlag();
	
	//make pairs for "breaks"
	cfgpairs.makeLoopBreakPairs(cfgpairs.newIndex());

	//create pair for loop beginning and statement after loop 
	cfgpairs.addPair(index, cfgpairs.getIndex());

	//pop loop from stack
	cfgpairs.popLoopFromStack();

	this->code_out += "\n" + indent + "/* " + cfgpairs.getIndex() + " */";	//add empty block after "for" loop
}


//parse "if" statement
void Parser::IfStat(string& stat_in, int tabs)
{
	string indent = makeIndent(tabs-1);
	string index = cfgpairs.getFlowIndex();

	smatch m;
	regex_match(stat_in, m, gc->ifelse_stat);

	this->code_out += indent + "/* " + index + " */ " + m[4].str() + "\n";		//add "if (...)" part to code
	
	//add "if" pair
	cfgpairs.newPair();
	Stat(m[2].str(), tabs+1);

	//save index and jump flag for last if-statement
	string ifstat_index = cfgpairs.getIndex();
	bool ifstat_jmp = cfgpairs.getJumpFlag();
	cfgpairs.resetJumpFlag();

	//if "else" part present
	if (m[3].str() != "") {	

		//add "else" part
		cfgpairs.addPair(index, cfgpairs.newIndex());
		this->code_out += "\n" + indent + "/* " + cfgpairs.getIndex() + " */ " + "else\n";
		
		//parse else-statement
		Stat(m[3].str(), tabs+1);

		//save index and jump flag for last else-statement
		string elsestat_index = cfgpairs.getIndex();
		bool elsestat_jmp = cfgpairs.getJumpFlag();
		cfgpairs.resetJumpFlag();

		//new index for statement after if-else block
		cfgpairs.newIndex();

		//add pair for last if-statement and statement after if-else block
		if (!ifstat_jmp) cfgpairs.addPair(ifstat_index, cfgpairs.getIndex());
		//add pair for last else-statement and statement after if-else block
		if (!elsestat_jmp) cfgpairs.addPair(elsestat_index, cfgpairs.getIndex());
	} 	
	else {
		//new index for statement after if-then block
		cfgpairs.newIndex();

		//add pair for last if-statement and statement after if-then block
		if (!ifstat_jmp) cfgpairs.addPair(ifstat_index, cfgpairs.getIndex());

		//add pair for if-then beginning and statement after if-then block
		cfgpairs.addPair(index, cfgpairs.getIndex());
	}

	this->code_out += "\n" + indent + "/* " + cfgpairs.getIndex() + " */";	//add empty block after if-else
}


//parse jump statement
void Parser::JumpStat(string& stat_in, int tabs)
{
	string indent = makeIndent(tabs-1);
	string index = cfgpairs.getFlowIndex();

	smatch m;
	if (regex_match(stat_in, m, gc->jump_return)) {			//return
		cfgpairs.addReturn();
	}
	else if (regex_match(stat_in, gc->jump_continue)) {		//continue
		cfgpairs.addContinue();
	}
	else if (regex_match(stat_in, gc->jump_break)) {		//break
		cfgpairs.addBreak();
	}
	else if (regex_match(stat_in, m, gc->jump_goto)) {		//goto
		cfgpairs.addGoto(m[1].str());	// m[1] - label name
	}

	this->code_out += indent + "/* " + index + " */ " + stat_in;
}


//parse label statement
void Parser::LblStat(string& stat_in, int tabs)
{
	string indent = makeIndent(tabs-1);
	string index = cfgpairs.getIndex();	
	string newindex = cfgpairs.newIndex(); //new label - new index

	//create pair with preceeding statement (if not jump)
	if (!cfgpairs.getJumpFlag()) cfgpairs.addPair(index, newindex);
	else cfgpairs.resetJumpFlag();

	smatch m;
	if (regex_match(stat_in, m, gc->lbl_case)) {		//if "case" statement
		this->code_out += indent + "/* " + newindex + " */ " + "case " + m[1].str() + ":\n";

		// link with switch
		cfgpairs.addCase();

		//parse statements after "case"
		Stat(m[2].str(), tabs);
	}
	else if (regex_match(stat_in, m, gc->lbl_def)) {		//if "default" statement
		this->code_out += indent + "/* " + newindex + " */ " + "default:\n";

		// link with switch
		cfgpairs.addDefault();

		//parse statements after "default" 
		Stat(m[1].str(), tabs);
	}
	else if (regex_match(stat_in, m, gc->lbl_label)) {
		//add label name and its new index to list
		cfgpairs.addLabel(m[1].str());		// m[1] - label name
		
		this->code_out += indent + "/* " + newindex + " */ " + m[1].str() + ":\n";
				
		//parse statements after label
		Stat(m[2].str(), tabs);
	}
}


//parse general statement
void Parser::Stat(string& stat_in, int tabs)
{
	if (stat_in != "") {
		if (regex_match(stat_in, gc->decl_stat)) {							//declaration statement
			DeclStat(stat_in, tabs);
		}
		else if (regex_match(stat_in, gc->expr_stat)) {						//expression statement
			ExprStat(stat_in, tabs);
		}
		else if (regex_match(stat_in, gc->comp_stat)) {						//compound statement
			CompStat(stat_in, tabs);
		}
		else if (regex_match(stat_in, gc->while_stat)) {					//while loop statement
			WhileStat(stat_in, tabs);
		}
		else if (regex_match(stat_in, gc->dowhile_stat)) {					//do-while loop statement
			DowhileStat(stat_in, tabs);
		}
		else if (regex_match(stat_in, gc->for_stat)) {						//for loop statement
			ForStat(stat_in, tabs);
		}
		else if (regex_match(stat_in, gc->ifelse_stat)) {					//if-else statement
			IfStat(stat_in, tabs);
		}
		else if (regex_match(stat_in, gc->switch_stat)) {					//switch statement
			SwitchStat(stat_in, tabs);
		}
		else if (regex_match(stat_in, gc->lbl_stat)) {						//label statement
			LblStat(stat_in, tabs);
		}
		else if (regex_match(stat_in, gc->jump_stat)) {						//jump statement
			JumpStat(stat_in, tabs);
		}
	}
}


//parse switch statement
void Parser::SwitchStat(string& stat_in, int tabs)
{
	string indent = makeIndent(tabs-1);
	string index = cfgpairs.getFlowIndex();

	smatch m;
	regex_match(stat_in, m, gc->switch_stat);

	this->code_out += indent + "/* " + index + " */ " + m[3].str() + "\n";		//add "switch (...)" part to code

	//add switch to stack
	cfgpairs.pushSwitchToStack();

	//parse switch statement
	Stat(m[2].str(), tabs+1);

	//link last switch statement with statement after switch
	if (!cfgpairs.getJumpFlag()) {
		cfgpairs.newPair();
	}
	else {
		cfgpairs.newIndex();
		cfgpairs.resetJumpFlag();
	}

	//make pairs for switch "breaks" and statement after switch
	cfgpairs.makeSwitchBreakPairs();

	// link switch beginning and statement after switch (if no default label present)
	if (!cfgpairs.isDefaultPresent()) {
		cfgpairs.addPair(index, cfgpairs.getIndex());
	}

	//pop switch from stack
	cfgpairs.popSwitchFromStack();

	//add empty index for statement after switch
	this->code_out += "\n" + indent + "/* " + cfgpairs.getIndex() + " */";
}


//parse "while" statement
void Parser::WhileStat(string& stat_in, int tabs)
{
	string indent = makeIndent(tabs-1);
	string index = cfgpairs.getFlowIndex();

	smatch m;
	regex_match(stat_in, m, gc->while_stat);

	this->code_out += indent + "/* " + index + " */ " + m[3].str() + "\n";		//add "while (...)" part to code

	//add loop to stack
	cfgpairs.pushLoopToStack();

	//add new pair for beginning and while-statement
	cfgpairs.newPair();

	//parse while statement
	Stat(m[2].str(), tabs+1);

	//make pairs for "continues"
	cfgpairs.makeLoopContinuePairs(index);

	//create pair for last loop statement and beginning of the loop
	if (!cfgpairs.getJumpFlag()) cfgpairs.addPair(cfgpairs.getIndex(), index);
	else cfgpairs.resetJumpFlag();

	//make pairs for "breaks"
	cfgpairs.makeLoopBreakPairs(cfgpairs.newIndex());

	//create pair for loop beginning and statement after loop 
	cfgpairs.addPair(index, cfgpairs.getIndex());

	//pop loop from stack
	cfgpairs.popLoopFromStack();

	this->code_out += "\n" + indent + "/* " + cfgpairs.getIndex() + " */";	//add empty block after "while" loop
}


//Main method - parse source code
void Parser::ParseCode()
{
	string functions = "";
	string statements = "";

	smatch m;
	if (code_in != "" && code_out == "") {
		if (regex_match(code_in, m, gc->program_code)) {			// input code is functions
			functions = m[1].str();

			sregex_iterator end;
			for (sregex_iterator iter(cbegin(functions), cend(functions), gc->func_def_spc); iter != end; iter++) {
				FuncDef((*iter)[1].str());
				break; // parse only first function
			}
		}
		else if (regex_match(code_in, m, gc->gl_statement)) {		// input code are statements
			statements = m[1].str();
			//parse all statements
			sregex_iterator end;
			for (sregex_iterator iter(cbegin(statements), cend(statements), gc->stat_space); iter != end; iter++) {
				Stat((*iter)[1].str(), 1);	//indent = 1 (for index correction)
				this->code_out += "\n";
			}
		}
	}
}