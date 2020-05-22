//////////////////////////////////////////////////////
// author Daniel Clarke 2013/14

#ifndef STATE_H
#define STATE_H

#include <string>
#include <vector>
#include <map>
#include "Trans_Input.h"

class State;
typedef std::vector<std::pair<Trans_Input, State*>> transTable;
typedef std::vector<std::pair<Trans_Input, State*>>::iterator transIter;

class State
{
public:

	State(bool, std::string, State*, State_Output*, State_Output*);

	void setindex(int);
	void setVisited(bool);
	void setNextState(Trans_Input, State*);
	void setOnEntryOutputs(State_Output*);
	void setOnDuringOutputs(State_Output*);

	//
	void setDepth(float);
	void setXPos(float);
	void setMod(float);
	void setChange(float);
	void setShift(float);
	void setGroupNum(unsigned int);
	void setParent(State*);
	void setThread(State*);
	void setAncestor(State*);
	void setLeftMostSib(State*);
	void setRightMostSib(State*);

	transTable getTransTable();
	State_Output* getOnEntryOutputs();
	State_Output* getOnDuringOutputs();

	int getTranTableSize();
	int getIndex();
	std::string getName();
	bool isSelectable();
	bool isVisited();

	//
	float getDepth();
	float getXPos();
	float getMod();
	float getChange();
	float getShift();
	unsigned int getGroupNum();
	State* getParent();
	State* getThread();
	State* getAncestor();
	State* getLeftMostSib();
	State* getRightMostSib();

	State* left();
	State* right();
	State* leftBrother();
	void leftMostSiblingCalc();
	void rightMostSiblingCalc();

	void removeStateTransition(std::pair<Trans_Input, State*>);

	bool checkTrans(Trans_Input, Trans_Input);
	bool containsTrans(Trans_Input);
	State* determineNextTransition(std::map<std::string, double>);
	bool interrogateCondition(std::map<std::string, double>, Trans_Input);

	std::string buildTransCopy(Trans_Input);
	std::vector<int> getManualStateIndexes();

private:

	// a state will maintain its own transition table to the states it can move to
	//in this case in the form of a vector
	int index;
	bool selectable;
	std::string stateName;

	transTable trans_table;
	State_Output* onEntryOutputs;
	State_Output* onDuringOutputs;

	//for minimisation
	bool visited;

	//for drawing
	float y;
	float x;
	float mod;
	float change;
	float shift;
	unsigned int number;
	State* parent;
	State* thread;
	State* ancestor;
	State* leftMostSib;
	State* rightMostSib;
};

#endif //STATE_H
