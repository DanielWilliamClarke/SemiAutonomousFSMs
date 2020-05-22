//////////////////////////////////////////////////////
// author Daniel Clarke 2013/14

#ifndef TRANS_INPUT_H
#define TRANS_INPUT_H

#include <string>

struct State_Output
{
	std::string channel;
	double value;
	State_Output* nextOutput;
};

struct Trans_Input 
{
	std::string channel;
	std::string opcode;
	// value is a string so that things like true/false can be entered, negative values and floating point numbers can be parsed
	double value;
	std::string logicOpCode;
	Trans_Input* nextCondtion;
};

struct Regex_Parse_Data
{
	Regex_Parse_Data(State_Output* output)
	    : StartStateOutputEntry(output), StartStateOutputDuring(output), stateOutputEntry(output), stateOutputDuring(output) {}
	std::string stateName;
	bool selectable;
	State_Output* StartStateOutputEntry;
	State_Output* StartStateOutputDuring;
	Trans_Input* inputCondition;
	State_Output* stateOutputEntry;
	State_Output* stateOutputDuring;
};

#endif //TRANS_INPUT_H