// This code is in the public domain - feel free to do anything you 
// wish with it.
//
// Eli Bendersky (spur4444@yahoo.com)
// 

#include <iostream>
#include <cassert>
#include <algorithm>
#include "nfa.h"


using namespace std;


/////////////////////////////////////////////////////////////////
//
// Implementation of the NFA class
//

//NFA::NFA(unsigned size_, state initial_, state final_)
//	: size(size_), initial(initial_), final(final_)
//{
//    assert(is_legal_state(initial));
//    assert(is_legal_state(final));
//    
//	vector<input> empty_row(size, NONE);
//	
//    // Initialize trans_table with an "empty graph", no transitions
//    // between its states
//    //
//    for (unsigned i = 0; i < size; ++i)
//        trans_table.push_back(empty_row);
//}
//

//NFA& NFA::operator=(const NFA& other)
//{
//	if (this == &other)
//		return *this;
//	
//	initial = other.initial;
//	final = other.final;
//	size = other.size;
//	trans_table = other.trans_table;
//	
//	return *this;
//}
//



void NFA::fill_states(const NFA& otherNFA)
{
	for (auto it = otherNFA.stateSet.cbegin(); it != otherNFA.stateSet.cend(); ++it)
    {
		stateSet.push_back(*it);
	}
}

//void NFA::show(void)
//{
//    cout << "This NFA has " << size << " states: 0 - " << size - 1 << endl;
//    cout << "The initial state is " << initial << endl;
//    cout << "The final state is " << final << endl << endl;
//    
//    for (state from = 0; from < size; ++from)
//    {
//        for (state to = 0; to < size; ++to)
//        {
//            input in = trans_table[from][to];
//            
//            if (in != NONE)
//            {
//                cout << "Transition from " << from << " to " << to << " on input ";
//                    
//                if (in == EPS)
//                {
//                    cout << "EPS" << endl;
//                }
//                else
//                {
//                    cout << in << endl;
//                }
//            }
//        }
//    }
//	
//	cout << endl;
//}

NFA::NFA()
{

}

// helper fucntions 

stateVec NFA::getInitialState()
{
	return initialState;
}

void NFA::setInitalState(stateVec vec)
{
	initialState = vec;
}

void NFA::addInitalState(State* s)
{
	initialState.push_back(s);
}

stateVec NFA::getFinalState()
{
	return finalState;
}

void NFA::setFinalState(stateVec vec)
{
	finalState = vec;
}

void NFA::addFinalState(State* s)
{
	finalState.push_back(s);
}

stateVec NFA::getStateVec()
{
	return stateSet;
}

void NFA::setStateSet(stateVec vec)
{
	stateSet = vec;
}

void NFA::addStateSet(State* s)
{
	stateSet.push_back(s);
}

unsigned NFA::getSize()
{
	return size;
}

void NFA::setSize(unsigned s)
{
	size = s;
}

std::string NFA::getType()
{
	return type;
}

void NFA::setType(std::string t)
{
	type = t;
}

void NFA::setNewInitial()
{
	std::vector<std::vector<State*>> temp;

	for(auto it = initialState.cbegin(); it != initialState.cend(); ++it)
		temp.push_back((*it)->getStateStates());

	// erase all data in the intial states, and place the new states in this vector
	for(auto it = initialState.begin(); it != initialState.end(); ++it)
	{
		for(auto it1 = stateSet.begin(); it1 != stateSet.end(); ++it1)
		{
			if((*it) == (*it1))
			{
				it1 = stateSet.erase(it1);
			}
		}
	}
	initialState.clear();

	for(auto it = temp.cbegin(); it != temp.cend(); ++it)
	{
		for(auto it1 = (*it).begin(); it1 != (*it).cend(); ++it1)
		{
			initialState.push_back(*it1);
		}
	}
}


/////////////////////////////////////////////////////////////////
//
// NFA building functions
//
// Using Thompson Construction, build NFAs from basic inputs or 
// compositions of other NFAs.
//


// Builds a basic, single input NFA
//
NFA* build_nfa_basic(input in)
{
	//construct new NFA
	NFA *basicNFA = new NFA();

	// construct two states
	// start state with transistion to next state
	State *startState = new State();
	State *nextState = new State();
	startState->setNextState(in, nextState);

	// put states into the nfa state set
	basicNFA->addStateSet(startState);
	basicNFA->addStateSet(nextState);

	// set start and final states for easy look up
	basicNFA->addInitalState(startState);
	basicNFA->addFinalState(nextState);
	basicNFA->setSize(basicNFA->getStateVec().size());
	basicNFA->setType("basic");
	// return the nfa
	return basicNFA;
}

// Builds an alternation of nfa1 and nfa2 (nfa1|nfa2)
//
NFA* build_nfa_alter(NFA *nfa1, NFA *nfa2)
{
	auto nfa1Init = nfa1->getInitialState();
	auto nfa1Fin = nfa1->getFinalState();
	auto nfa2Init = nfa2->getInitialState();
	auto nfa2Fin = nfa2->getFinalState();

	//take the initial state of nf1, giving the state transition of the initial state of nfa2
	NFA *alterNFA = new NFA();
	// get the transitions from nf2 and give them to nfa1 initial
	alterNFA->fill_states(*nfa1);
	alterNFA->setInitalState(nfa1Init);
	auto alterInit = alterNFA->getInitialState();

	std::vector<std::vector<char>> temp;
	for(auto it = nfa2Init.begin(); it != nfa2Init.end(); ++it)
		temp.push_back((*it)->getStateTransitions());

	// check transistion table for states make these states the new intial states of nfa2 
	nfa2->setNewInitial();
	nfa2Init = nfa2->getInitialState();

	alterNFA->fill_states(*nfa2);

	// put final states of new nfa to new intials of nfa2
	for(auto it = alterInit.begin(); it != alterInit.end(); ++it)
	{
		for(std::pair<StateIter, std::vector<char>::iterator> it2(nfa2Init.begin(), temp[0].begin()); 
			it2.first != nfa2Init.end(); ++it2.first, ++it2.second)
		{
			(*it)->setNextState((*it2.second), (*it2.first));
		}
	}

	for(auto it = nfa1Fin.cbegin(); it != nfa1Fin.cend(); ++it)
		alterNFA->addFinalState(*it);
	
	for(auto it = nfa2Fin.cbegin(); it != nfa2Fin.cend(); ++it)
		alterNFA->addFinalState(*it);

	alterNFA->setSize(alterNFA->getStateVec().size());
	alterNFA->setType("alter");

	// delete old nfas from memory
	delete nfa1;
	delete nfa2;

	return alterNFA;
}

// Builds a concatenation of nfa1 and nfa2 (nfa1nfa2)
//
NFA* build_nfa_concat(NFA* nfa1, NFA* nfa2)
{
	auto nfa1Init = nfa1->getInitialState();
	auto nfa1Fin = nfa1->getFinalState();
	auto nfa2Init = nfa2->getInitialState();
	auto nfa2Fin = nfa2->getFinalState();

	//take final state of nfa1 and make it nfa2 start state
	//merge the two nfas
	// construct new NFA
	NFA *concatNFA = new NFA();

	//put all states of nfa1 into the new nfa
	concatNFA->fill_states(*nfa1);
	concatNFA->setInitalState(nfa1->getInitialState());

	// clear and push on new final states
	if(nfa1->getType() == "star")
		for(auto it = nfa1Fin.cbegin(); it != nfa1Fin.cend(); ++it)
			concatNFA->addFinalState(*it);
	else
		concatNFA->setFinalState(nfa1->getFinalState());

	auto concatInit = concatNFA->getInitialState();
	auto concatFin = concatNFA->getFinalState();

	//get transistion of start state put it into the final state of nfa1
	//must also consider the number of intial and final states each nfa has
	//apply the final states transitions to all initial states
	std::vector<std::vector<char>> temp;

	for(auto it = nfa2Init.cbegin(); it != nfa2Init.cend(); ++it)
		temp.push_back((*it)->getStateTransitions());

	//remove the initial state from nfa2 and make the next state the intial
	//push in the states from nfa2 into our new nfa
	//apply the transitions from the final state from nfa1 to start of nfa2

	// check transistion table for states make these states the new intial states of nfa2 
	nfa2->setNewInitial();
	concatNFA->fill_states(*nfa2);
	nfa2Init = nfa2->getInitialState();

	// put final states of new nfa to new intials of nfa2
	for(std::pair<StateIter, std::vector<char>::iterator> it(nfa2Init.begin(), temp[0].begin());
		it.first != nfa2Init.end(); ++it.first, ++it.second)
	{
		for(auto it1 = concatFin.begin(); it1 != concatFin.end(); ++it1)
		{
			(*it1)->setNextState((*it.second), (*it.first));
		}
	}

	//this is to account for the 0 or more instances of transition 'x'
	if(nfa1->getType() == "star")
	{
		for(std::pair<StateIter, std::vector<char>::iterator> it(nfa2Init.begin(), temp[0].begin());
			it.first != nfa2Init.end(); ++it.first, ++it.second)
		{
			for(auto it1 = concatInit.begin(); it1 != concatInit.end(); ++it1)
			{
				(*it1)->setNextState((*it.second), (*it.first));
			}
		}
	}

	// clear and push on new final states
	if(nfa2->getType() == "star")
		for(auto it = nfa2Fin.cbegin(); it != nfa2Fin.cend(); ++it)
			concatNFA->addFinalState(*it);
	else
		concatNFA->setFinalState(nfa2->getFinalState());

	concatNFA->setSize(concatNFA->getStateVec().size());
	concatNFA->setType("concat");

	delete nfa1;
	delete nfa2;

	return concatNFA;
}


// Builds a star (kleene closure) of nfa (nfa*)
//
NFA* build_nfa_star(NFA* nfa)
{
	// not adding any new states
	// we are just adding new transistions to nfa

	auto nfaInit = nfa->getInitialState();
	auto nfaFin = nfa->getFinalState();

	//get the state transistions from the start of nfa
	std::vector<std::vector<char>> tempTrans;
	//get the states from the start of nfa
	std::vector<std::vector<State*>> tempStates;

	for(auto it1 = nfaInit.cbegin(); it1 != nfaInit.cend(); ++it1)
	{
		tempTrans.push_back((*it1)->getStateTransitions());
		tempStates.push_back((*it1)->getStateStates());
	}

	// put final states of new nfa to new intials of nfa2
	for(auto it = nfaFin.begin(); it != nfaFin.end(); ++it)
	{
		for(std::pair<std::vector<char>::iterator, StateIter> it2(tempTrans[0].begin(), tempStates[0].begin()); it2.first != tempTrans[0].end(); ++it2.first, ++it2.second)
		{
			// check transisitions if it already exists then skip
			if(!(*it)->containsTrans((*it2.first)))
			{
				(*it)->setNextState((*it2.first), (*it2.second));
			}
		}	
	}

	nfa->setType("star");

	return nfa;
}






