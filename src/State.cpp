//////////////////////////////////////////////////////
// author Daniel Clarke 2013/14

#include <sstream>
#include <iomanip>
#include "State.h"

State::State(bool select, std::string name, State* par, State_Output* entry, State_Output* during)
{
	visited = false;
	index = rand() % 100 + 1;
	selectable = select;
	stateName = name;
	onEntryOutputs = entry;
	onDuringOutputs = during;

	//x and y coord for drawing
	y = 0.0f;
	x = -1.0f;

	//parernt will be the left most state pointing at this state
	parent = par;
	thread = nullptr;
	ancestor = this;
	leftMostSib = nullptr;
	rightMostSib = nullptr;

	//for calculating x and y
	mod = 0;
	change = 0;
	shift = 0;

	// this value is its number in its group of siblings
	number = 0;
}

int State::getIndex()
{
	return index;
}

void State::setindex(int idx)
{
	index = idx;
}

float State::getDepth()
{
	return y;
}

void State::setDepth(float depth)
{
	y = depth;
}

float State::getXPos()
{
	return x;
}

void State::setXPos(float xPos)
{
	x = xPos;
}

transTable State::getTransTable()
{
	return trans_table;
}

int State::getTranTableSize()
{
	return trans_table.size();
}

State_Output* State::getOnEntryOutputs()
{
	return onEntryOutputs;
}

State_Output* State::getOnDuringOutputs()
{
	return onDuringOutputs;
}

std::string State::getName()
{
	return stateName;
}

bool State::isSelectable()
{
	return selectable;
}

void State::setVisited(bool visit)
{
	visited = visit;
}

bool State::isVisited()
{
	return visited;
}

void State::setNextState(Trans_Input in, State* nextState)
{
	trans_table.push_back(std::make_pair(in, nextState));
}

bool State::containsTrans(Trans_Input trans)
{
	bool contains = false;
	Trans_Input stateTrans;
	for(transIter it = trans_table.begin(); it != trans_table.end(); ++it)
	{
		if(checkTrans(it->first, stateTrans))
		{
			return true;
		}
	}
	return contains;
}

void State::removeStateTransition(std::pair<Trans_Input, State*> stateTrans)
{
	for(transIter it = trans_table.begin(); it != trans_table.end(); ++it)
	{
		if(checkTrans(it->first, stateTrans.first) && it->second == stateTrans.second)
		{
			it = trans_table.erase(it);
			return;
		}
	}
}

bool State::checkTrans(Trans_Input left, Trans_Input right)
{
	bool contains = false;

	if(left.channel == right.channel && left.opcode == right.opcode && left.value == right.value) 
	{
		contains = true;
	}
	else
	{ 
		// if a single condition is different the transition is then different 
		// no recursion here so the false goes straight back
		return false;
	}

	if(left.nextCondtion != nullptr && right.nextCondtion != nullptr)
	{
		left = *left.nextCondtion;
		right = *right.nextCondtion;
		contains = checkTrans(left, right);
	}
	return contains;
}

State* State::determineNextTransition(std::map<std::string, double> inputChannels)
{
	for(transIter it = trans_table.begin(); it != trans_table.end(); ++it)
	{
		// get the value from the input channel list, 
		if(interrogateCondition(inputChannels, it->first))
		{
			return it->second;
		}
	}

	//otherwise no transition made return current state
	return this;
}

bool State::interrogateCondition(std::map<std::string, double> inputChannels, Trans_Input currentTrans)
{
	bool evaluation = false;

	double testValue = inputChannels[currentTrans.channel];

	if (currentTrans.opcode == "==")
	{
		evaluation = testValue == currentTrans.value ? true : false;
	}
	else if (currentTrans.opcode == "!=")
	{
		evaluation = testValue != currentTrans.value ? true : false;
	}
	else if (currentTrans.opcode == "<")
	{
		evaluation = testValue < currentTrans.value ? true : false;
	}
	else if (currentTrans.opcode == ">")
	{
		evaluation = testValue > currentTrans.value ? true : false;
	}
	else if (currentTrans.opcode == "<=")
	{
		evaluation = testValue <= currentTrans.value ? true : false;
	}
	else if (currentTrans.opcode == ">=")
	{
		evaluation = testValue >= currentTrans.value ? true : false;
	}
	
	if(currentTrans.logicOpCode == "&&")
	{
		evaluation = evaluation && interrogateCondition(inputChannels, *currentTrans.nextCondtion) ? true : false;
	}
	else if (currentTrans.logicOpCode == "||")
	{
		evaluation = evaluation || interrogateCondition(inputChannels, *currentTrans.nextCondtion) ? true : false;
	}

	return evaluation;
}

//
float State::getMod()
{
	return mod;
}

void State::setMod(float m)
{
	mod = m;
}

float State::getChange()
{
	return change;
}

void State::setChange(float c)
{
	change = c;
}

float State::getShift()
{
	return shift;
}

void State::setShift(float s)
{
	shift = s;
}

unsigned int State::getGroupNum()
{
	return number;
}

void State::setGroupNum(unsigned int gn)
{
	number = gn;
}

State* State::getParent()
{
	return parent;
}

void State::setParent(State* p)
{
	parent = p;
}

State* State::getThread()
{
	return thread;
}

void State::setThread(State* t)
{
	thread = t;
}

State* State::getAncestor()
{
	return ancestor;
}

void State::setAncestor(State* a)
{
	ancestor = a;
}

State* State::getLeftMostSib()
{
	return leftMostSib;
}

void State::setLeftMostSib(State* s)
{
	leftMostSib = s;
}

State* State::getRightMostSib()
{
	return rightMostSib;
}

void State::setRightMostSib(State* r)
{
	rightMostSib = r;
}

State* State::leftBrother()
{
	State* bro = nullptr;
	if(parent != nullptr)
	{
		transTable parTrans = parent->trans_table;

		for(transIter it = parTrans.begin(); it != parTrans.end(); ++it)
		{
			if(it->second == this)
			{
				return bro;
			}
			else
			{
				bro = it->second;
			}
		}
	}
	return bro;
}

void State::leftMostSiblingCalc()
{
	if(number == 1)
	{
		//I am left most
		leftMostSib = nullptr;
		return;
	}

	transTable parTrans = parent->trans_table;

	for(transIter it = parTrans.begin(); it != parTrans.end(); ++it)
	{
		leftMostSib = it->second;
		return;
	}
}

void State::rightMostSiblingCalc()
{
	if(number == parent->trans_table.size())
	{
		//I am right most
		rightMostSib = nullptr;
		return;
	}

	transTable parTrans = parent->trans_table;

	for(transIter it = parTrans.begin(); it != parTrans.end(); ++it)
	{
		rightMostSib = it->second;
	}
}

State* State::left()
{
	if(trans_table.size() == 0 
		|| trans_table.at(0).second == this)
	{
		return thread; 
	}
	return trans_table.at(0).second;
}

State* State::right()
{
	if(this == nullptr
		|| trans_table.size() == 0
		|| trans_table.at(0).second->rightMostSib == nullptr
		|| trans_table.at(0).second->rightMostSib != nullptr && trans_table.at(0).second->rightMostSib == this)
	{
		return thread;
	}

	return trans_table.at(0).second->rightMostSib;
}

std::string State::buildTransCopy(Trans_Input input)
{
	std::string temp;

	temp += input.channel;
	temp += input.opcode;

	std::stringstream ss;
	ss << std::fixed << std::setprecision(2) << input.value;
	temp += ss.str();
						
	if(input.nextCondtion != nullptr)
	{
		temp += input.logicOpCode;
		temp += buildTransCopy(*input.nextCondtion);
	}
	return temp;
}

std::vector<int> State::getManualStateIndexes()
{
	std::vector<int> manualStatesIndex;
	for(transIter it = trans_table.begin(); it != trans_table.end(); ++it)
	{
		if(it->second->isSelectable() && this != it->second)
		{
			manualStatesIndex.push_back(it->second->getIndex());
		}
	}
	return manualStatesIndex;
}
