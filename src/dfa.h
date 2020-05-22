//////////////////////////////////////////////////////
// author Daniel Clarke 2013/14

#ifndef DFA_H
#define DFA_H

#include <map>
#include "State.h"

typedef std::vector<State*>::iterator StateIter;
typedef std::vector<State*> stateVec;

class DFA
{
public:
    
	DFA();

	unsigned getSize();
	int getType();
	stateVec getInitialState();
	stateVec getFinalState();
	stateVec getStateVec();

	void fill_states(DFA& otherDFA);
	void setInitalState(stateVec);
	void setNewInitialState();
	void setFinalState(stateVec);
	void setStateSet(stateVec);
	void setSize(unsigned);
	void setType(int);

	void addInitalState(State*);
	void addFinalState(State*);
	void addStateSet(State*);

	void removeState(State*);
	void fixFinalState(State*);
	void clearFinalState();
	void unvisitStates();
	void numberStates();

	void calculateDepths(State*);

	//minimisation algorithm
	void minimise_DFA(State*, State*, transTable);
	
	std::map<std::string, double> getInputChannels();
	std::map<std::string, double> getOutputChannels();
	void setInputChannels(std::map<std::string, double>);
	void setOutputChannels(std::map<std::string, double>);

	//drawing algorithm
	State* buchhiem(State*);
	int getNumberOfConditions(Trans_Input, int);
	int numberOfCondtions;

private:

	int type;

	//sets of initial and final states
    std::vector<State*> initialState;
    std::vector<State*> finalState;
	//a set of all states
	std::vector<State*> stateSet;
	// total number of states in the DFA
    unsigned size;

	std::map<std::string, double> inputChannels;
	std::map<std::string, double> outputChannels;

	//drawing functions
	State* firstWalk(State*, float);
	float secondWalk(State*, float, float);
	void thirdWalk(State*, float);
	State* apportion(State*, State*, float);
	void moveSubTree(State*, State*, float);
	void executeShifts(State*);
	State* ancestor(State*, State*,  State*);
};
#endif // DFA_H