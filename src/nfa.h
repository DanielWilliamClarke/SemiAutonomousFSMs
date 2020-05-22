// This code is in the public domain - feel free to do anything you
// wish with it.
//
// Eli Bendersky (spur4444@yahoo.com)
//

#ifndef NFA_H
#define NFA_H

#include <vector>
#include <map>
#include <assert.h>
#include "State.h"

// the state must become an object of type state
// state can contain entry, during and exit output parameters
// must create a vector of states
//typedef unsigned state;

// the input must become a map object
// must include for timing
//typedef char input;
//<channel><opcode><value>
//typedef Trans_Input* input;
typedef char input;
typedef std::vector<State *>::iterator StateIter;
typedef std::vector<State *> stateVec;
enum
{
	EPS = -1,
	NONE = 0
};

class NFA
{
public:
	NFA();

	NFA &operator=(const NFA &other);

	void fill_states(const NFA &otherNFA);

	void setNewInitial();

	void show(void);

	stateVec getInitialState();
	stateVec getFinalState();
	stateVec getStateVec();
	unsigned getSize();
	std::string getType();

	void setInitalState(stateVec);
	void setFinalState(stateVec);
	void setStateSet(stateVec);
	void setSize(unsigned);
	void setType(std::string);

	void addInitalState(State *);
	void addFinalState(State *);
	void addStateSet(State *);

private:
	std::string type;
	//sets of initial and final states
	std::vector<State *> initialState;
	std::vector<State *> finalState;
	//a set of all states
	std::vector<State *> stateSet;
	// total number of states in the nfa
	unsigned size;
};

// NFA building functions
//
NFA *build_nfa_basic(input in);
NFA *build_nfa_alter(NFA *nfa1, NFA *nfa2);
NFA *build_nfa_concat(NFA *nfa1, NFA *nfa2);
NFA *build_nfa_star(NFA *nfa);

#endif // NFA_H
