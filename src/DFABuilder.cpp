//////////////////////////////////////////////////////
// author Daniel Clarke 2013/14 
// based on code by Eli Bendersky (spur4444@yahoo.com)

#include "DFABuilder.h"

DFABuilder::DFABuilder(std::string toParse)
{
	try
	{
		// parsing algorithm
		Regex_parse* rp = new Regex_parse(toParse);
		// construction algorithm
		dfa = tree_to_dfa(rp->getTree());
		dfa->numberOfCondtions = rp->numberOfCondtions;
		// minimisation algorithm
		minimiseAndFix();

		dfa->numberStates();

		//for the opc server 
		dfa->setInputChannels(rp->getInputChannels());
		dfa->setOutputChannels(rp->getOutputChannels());

		delete rp;
	}
	catch(std::exception& ex)
	{
		throw ex;
	}
}

DFA* DFABuilder::getDFA()
{
	return dfa;
}

DFA* DFABuilder::tree_to_dfa(parse_node* tree)
{


	assert(tree);
	
	switch (tree->type)
	{
	case BASIC:
		return build_DFA_basic(tree->data);
	case ALTER:
		return build_DFA_alter(tree_to_dfa(tree->left), tree_to_dfa(tree->right));
	case CONCAT:
		return build_DFA_concat(tree_to_dfa(tree->left), tree_to_dfa(tree->right));
	case STAR:
		return build_DFA_star(tree_to_dfa(tree->left));
	case PLUS:
		return build_DFA_plus(tree_to_dfa(tree->left));
	default:
		//assert(0);
		return nullptr;
	}
}

void DFABuilder::minimiseAndFix()
{
	// Minimise
	dfa->numberStates();
	dfa->minimise_DFA(dfa->getInitialState().at(0), nullptr, dfa->getInitialState().at(0)->getTransTable());

	// Fix resultant FSM
	dfa->unvisitStates();
	dfa->clearFinalState();
	dfa->fixFinalState(dfa->getInitialState().at(0));
	dfa->setSize(dfa->getStateVec().size());
	dfa->unvisitStates();

	// for drawing calculation
	dfa->calculateDepths(dfa->getInitialState().at(0));
	dfa->unvisitStates();
	dfa->buchhiem(dfa->getInitialState().at(0));
	dfa->unvisitStates();
}

// Builds a basic, single input DFA
//
DFA* DFABuilder::build_DFA_basic(Regex_Parse_Data* in)
{
	//construct new DFA
	DFA *basicDFA = new DFA();

	// construct two states
	// start state with transistion to next state
	State *startState = new State(false, "Start", nullptr, in->StartStateOutputEntry, in->StartStateOutputDuring);
	State *nextState = new State(in->selectable,  in->stateName, startState, in->stateOutputEntry, in->stateOutputDuring);

	startState->setNextState(*in->inputCondition, nextState);

	// put states into the DFA state set
	basicDFA->addStateSet(startState);
	basicDFA->addStateSet(nextState);

	// set start and final states for easy look up
	basicDFA->addInitalState(startState);
	basicDFA->addFinalState(nextState);
	basicDFA->setSize(basicDFA->getStateVec().size());
	basicDFA->setType(BASIC);
	// return the DFA
	return basicDFA;
}

// Builds an alternation of DFA1 and DFA2 (DFA1|DFA2)
//
DFA* DFABuilder::build_DFA_alter(DFA *DFA1, DFA *DFA2)
{
	stateVec DFA1Init = DFA1->getInitialState();
	stateVec DFA1Fin = DFA1->getFinalState();
	stateVec DFA2Init = DFA2->getInitialState();
	stateVec DFA2Fin = DFA2->getFinalState();

	//take the initial state of nf1, giving the state transition of the initial state of DFA2
	DFA *alterDFA = new DFA();
	// get the transitions from nf2 and give them to DFA1 initial
	alterDFA->fill_states(*DFA1);
	alterDFA->setInitalState(DFA1Init);
	auto alterInit = alterDFA->getInitialState();

	//each DFA will have a single initial state at the start
	transTable trans = DFA2Init.at(0)->getTransTable();

	DFA2->setNewInitialState();
	DFA2Init = DFA2->getInitialState();
	alterDFA->fill_states(*DFA2);

	// put final states of new DFA to new intials of DFA2
	for(StateIter it = alterInit.begin(); it != alterInit.end(); ++it)
	{
		for(transIter it1 = trans.begin(); it1 != trans.end(); ++it1)
		{
			(*it)->setNextState((it1->first), (it1->second));
			//reset parent of the node
			if(it == alterInit.begin())
			{
				it1->second->setParent(*it);
			}
		}
	}

	// will have multiple final states
	for(StateIter it = DFA1Fin.begin(); it != DFA1Fin.end(); ++it)
	{
		alterDFA->addFinalState(*it);
	}
	
	for(StateIter it = DFA2Fin.begin(); it != DFA2Fin.end(); ++it)
	{
		alterDFA->addFinalState(*it);
	}

	alterDFA->setSize(alterDFA->getStateVec().size());
	alterDFA->setType(ALTER);

	// delete old DFAs from memory
	delete DFA1;
	delete DFA2;

	return alterDFA;
}

// Builds a concatenation of DFA1 and DFA2 (DFA1DFA2)
//
DFA* DFABuilder::build_DFA_concat(DFA* DFA1, DFA* DFA2)
{
	stateVec DFA1Init = DFA1->getInitialState();
	stateVec DFA1Fin = DFA1->getFinalState();
	stateVec DFA2Init = DFA2->getInitialState();
	stateVec DFA2Fin = DFA2->getFinalState();

	//take final state of DFA1 and make it DFA2 start state
	//merge the two DFAs
	// construct new DFA
	DFA *concatDFA = new DFA();

	//put all states of DFA1 into the new DFA
	concatDFA->fill_states(*DFA1);
	concatDFA->setInitalState(DFA1->getInitialState());

	// push on new final states
	if(DFA1->getType() == STAR) 
	{
		for(StateIter it = DFA1Fin.begin(); it != DFA1Fin.end(); ++it)
		{
			concatDFA->addFinalState(*it);
		}
	}
	else
	{
		concatDFA->setFinalState(DFA1->getFinalState());
	}

	stateVec concatInit = concatDFA->getInitialState();
	stateVec concatFin = concatDFA->getFinalState();

	//get transistion of start state put it into the final state of DFA1
	//must also consider the number of intial and final states each DFA has
	//apply the final states transitions to all initial states

	//each DFA will have a single initial state at the start
	transTable trans = DFA2Init.at(0)->getTransTable();

	//remove the initial state from DFA2 and make the next state the intial
	//push in the states from DFA2 into our new DFA
	//apply the transitions from the final state from DFA1 to start of DFA2

	// check transistion table for states make these states the new intial states of DFA2 
	DFA2->setNewInitialState();
	concatDFA->fill_states(*DFA2);
	DFA2Init = DFA2->getInitialState();

	for(StateIter it = concatFin.begin(); it != concatFin.end(); ++it)
	{
		for(transIter it1 = trans.begin(); it1 != trans.end(); ++it1)
		{
			(*it)->setNextState((it1->first), (it1->second));
			//reset parent of the node
			if(it == concatFin.begin())
			{
				it1->second->setParent(*it);
			}
		}
	}	
		
	if(DFA1->getType() == STAR)
	{
		for(StateIter it = concatInit.begin(); it != concatInit.end(); ++it)
		{
			for(transIter it1 = trans.begin(); it1 != trans.end(); ++it1)
			{
				if(!(*it)->containsTrans(it1->first))
				{
					(*it)->setNextState((it1->first), (it1->second));
				}
			}
		}
	}

	// clear and push on new final states
	if(DFA2->getType() == STAR) 
	{
		for(StateIter it = DFA2Fin.begin(); it != DFA2Fin.end(); ++it)
		{
			concatDFA->addFinalState(*it);
		}
	}
	else
	{
		concatDFA->setFinalState(DFA2->getFinalState());
	}

	concatDFA->setSize(concatDFA->getStateVec().size());
	concatDFA->setType(CONCAT);

	delete DFA1;
	delete DFA2;

	return concatDFA;
}


// Builds a star (kleene closure) of DFA (DFA*)
//
DFA* DFABuilder::build_DFA_star(DFA* DFA)
{
	// not adding any new states
	// we are just adding new transitions to DFA

	stateVec DFAInit = DFA->getInitialState();
	stateVec DFAFin = DFA->getFinalState();

	//each DFA will have a single initial state at the start
	transTable trans = DFAInit.at(0)->getTransTable();

	// put final states of new DFA to new intials of DFA2
	for(StateIter it = DFAFin.begin(); it != DFAFin.end(); ++it)
	{
		for(transIter it1 = trans.begin(); it1 != trans.end(); ++it1)
		{
			// check transitions if it already exists then skip
			if(!(*it)->containsTrans((it1->first)))
			{
				(*it)->setNextState((it1->first), (it1->second));
			}
		}	
	}

	DFA->setType(STAR);

	return DFA;
}

// Builds a plus (kleene closure) of DFA (DFA+)
//
DFA* DFABuilder::build_DFA_plus(DFA* DFA)
{
	// its basically the same as building a star dfa but the type is changed
	//so that it keeps the 1 or more transitions aspect which if left as STAR will have zero or more transitions when concatenated
	DFA = build_DFA_star(DFA);
	DFA->setType(PLUS);
	return DFA;
}