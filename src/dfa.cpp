//////////////////////////////////////////////////////
// author Daniel Clarke 2013/14

#include "dfa.h"

DFA::DFA()
{
	numberOfCondtions = 0;
}

stateVec DFA::getInitialState()
{
	return initialState;
}

void DFA::setInitalState(stateVec vec)
{
	initialState = vec;
}

void DFA::addInitalState(State* s)
{

	initialState.push_back(s);
}

stateVec DFA::getFinalState()
{
	return finalState;
}

void DFA::setFinalState(stateVec vec)
{
	finalState = vec;
}

void DFA::addFinalState(State* s)
{
	//just a check to see if it exists in the set
	for(StateIter it = finalState.begin(); it != finalState.end(); ++it)
	{
		if((*it)->getIndex() == s->getIndex())
		{
			return;
		}
	}

	finalState.push_back(s);
}

stateVec DFA::getStateVec()
{
	return stateSet;
}

void DFA::setStateSet(stateVec vec)
{
	stateSet = vec;
}

void DFA::addStateSet(State* s)
{
	stateSet.push_back(s);
}

unsigned DFA::getSize()
{
	return size;
}

void DFA::setSize(unsigned s)
{
	size = s;
}

int DFA::getType()
{
	return type;
}

void DFA::setType(int t)
{
	type = t;
}

void DFA::fill_states(DFA& otherDFA)
{
	for (StateIter it = otherDFA.stateSet.begin(); it != otherDFA.stateSet.end(); ++it)
    {
		stateSet.push_back(*it);
	}
}

void DFA::setNewInitialState()
{
	transTable temp = initialState.at(0)->getTransTable();

	// erase all data in the intial states, and place the new states in this vector
	for(StateIter it = initialState.begin(); it != initialState.end(); ++it)
	{
		for(StateIter it1 = stateSet.begin(); it1 != stateSet.end(); ++it1)
		{
			if((*it) == (*it1))
			{
				delete (*it);
				it1 = stateSet.erase(it1);
			}
		}
	}
	initialState.clear();

	for(transIter it = temp.begin(); it != temp.end(); ++it)
	{
		initialState.push_back(it->second);
	}
}

void DFA::removeState(State* toDelete)
{
	for(StateIter it = stateSet.begin(); it != stateSet.end(); ++it)
	{
		if((*it) == toDelete)
		{
			it = stateSet.erase(it);
			return;
		}
	}
}

void DFA::fixFinalState(State* state)
{
	state->setVisited(true);
	transTable stateTrans = state->getTransTable();

	if(stateTrans.size() == 0)
	{
		addFinalState(state);
	}

	for(transIter it1 = stateTrans.begin(); it1 != stateTrans.end(); ++it1)
	{
		if(it1->second->isVisited() && stateTrans.size() == 1)
		{
			addFinalState(it1->second);
		}
		if(!it1->second->isVisited())
		{
			fixFinalState(it1->second);
		}
	}

}

void DFA::unvisitStates()
{
	for(StateIter it = stateSet.begin(); it != stateSet.end(); ++it)
	{
		(*it)->setVisited(false);
	}
}

void DFA::clearFinalState()
{
	finalState.clear();
}

void DFA::numberStates()
{
	int idx = 0;
	for(StateIter it = stateSet.begin(); it != stateSet.end(); ++it)
	{
		(*it)->setindex(idx);
		idx++;
	}
}

void DFA::calculateDepths(State* state)
{
	transTable stateTrans = state->getTransTable();

	int subGroupID = 0;
	state->setVisited(true);

	for(transIter it1 = stateTrans.begin(); it1 != stateTrans.end(); ++it1)
	{
		if(!it1->second->isVisited())
		{
			if(it1->second->getDepth() != state->getDepth() || it1->second->getDepth() == 0)
			{
				it1->second->setDepth(state->getDepth() + (400 * getNumberOfConditions(it1->first, 0)));
			}
		}
		it1->second->setGroupNum(++subGroupID);
		it1->second->leftMostSiblingCalc();
		it1->second->rightMostSiblingCalc();
	}

	for(transIter it1 = stateTrans.begin(); it1 != stateTrans.end(); ++it1)
	{
		if(!it1->second->isVisited())
		{
			calculateDepths(it1->second);
		}
	}
}

int DFA::getNumberOfConditions(Trans_Input input, int size)
{
	++size;

	if(input.nextCondtion != nullptr)
	{
		return getNumberOfConditions(*input.nextCondtion, size);
	}
	return size;
}

void DFA::minimise_DFA(State* state, State* fixState, transTable fixTrans)
{
	state->setVisited(true);
	int occurred = 0;
	std::string temp;
	std::map<std::string, std::pair<std::pair<Trans_Input, State*>, int>> occurances;
	transTable stateTrans = state->getTransTable();
	State* branchState = nullptr;

	//tests for duplicate trans
	for(transIter it1 = stateTrans.begin(); it1 != stateTrans.end(); ++it1)
	{
		//make note of a state with more than one transition
		if(stateTrans.size() > 1)
		{
			branchState = state;
		}

		//apply fix to state whoes transition state was deleted
		if (it1->second->getIndex() < 0)
		{
			state->setNextState(it1->first, fixState);
			state->removeStateTransition(*it1);
		}

		for(transIter it2 = stateTrans.begin(); it2 != stateTrans.end(); ++it2)
		{
			if(state->checkTrans(it1->first, it2->first))
			{
				temp = it2->second->buildTransCopy(it2->first);
				occurances[temp] = std::make_pair(std::make_pair(it2->first, it2->second), ++occurred);

				if(occurances[temp].second > 1)
				{
					if(it2->second->getIndex() > 0 && it2->second->getIndex() < 101)
					{
						//get transitions of the to be merged state
						transTable toBeMergedStateTrans = it2->second->getTransTable();
						fixTrans = toBeMergedStateTrans;

						//merge them into the combine state
						for(transIter it3 = toBeMergedStateTrans.begin(); it3 != toBeMergedStateTrans.end(); ++it3)
						{
							if((!it1->second->containsTrans(it3->first) && !it3->second->isVisited())
								&& it3->second != it2->second && it3->second != it1->second)
							{
								state->setNextState(it3->first, it3->second);
								//fix parent node
							}
							it3->second->setParent(it1->second);
						}
					}	
					else
					{
						//merge them into the combine state
						for(transIter it3 = fixTrans.begin(); it3 != fixTrans.end(); ++it3)
						{
							if((it3->second->getIndex() > 0 && it3->second->getIndex() < 101) &&
								(!state->containsTrans(it3->first) && !it3->second->isVisited()) 
								&& it3->second != it2->second && it3->second != it1->second)
							{
								state->setNextState(it3->first, it3->second);
							}
							it3->second->setParent(state);
						}
					}

					// erase original transition and delete merged state
					state->removeStateTransition(*it2);

					if(it1->second != it2->second)
					{
						removeState(it2->second);
					}
					
					stateTrans = state->getTransTable();
										
					//re validate the iterators
					it2 = stateTrans.begin();
					it1 = stateTrans.begin();

					//decrement the number of occurances 
					occurances.erase(temp);
				}
			}	
		}
		occurred = 0;
	}
	
	for(transIter it1 = stateTrans.begin(); it1 != stateTrans.end(); ++it1)
	{
		if(!it1->second->isVisited())
		{
			minimise_DFA(it1->second, branchState, fixTrans);
		}
	}
}

std::map<std::string, double> DFA::getInputChannels()
{
	return inputChannels;
}

std::map<std::string, double> DFA::getOutputChannels()
{
	return outputChannels;
}

void DFA::setInputChannels(std::map<std::string, double> inputs)
{
	inputChannels = inputs;
}

void DFA::setOutputChannels(std::map<std::string, double> outputs)
{
	outputChannels = outputs;
}

// Plotting Algorithm Translated from BillMill

State* DFA::buchhiem(State* state)
{
	State* dt = firstWalk(state, 300);
	unvisitStates();
	float min = secondWalk(dt, 0, NULL);
	if(min < 0)
	{
		unvisitStates();
		thirdWalk(dt, -min);
	}
	return dt;
}

State* DFA::firstWalk(State* state, float distance)
{
	state->setVisited(true);
	State* defaultAncestor = nullptr;
	if(state->getTransTable().size() == 0)
	{
		if(state->getLeftMostSib() != nullptr)
		{
			state->setXPos(state->leftBrother()->getXPos() + distance);
		}
		else
		{
			state->setXPos(0);
		}
	}
	else
	{
		defaultAncestor = state->getTransTable().at(0).second;

		transTable trans = state->getTransTable();

		for(transIter it = trans.begin(); it != trans.end(); ++it)
		{
			if(!it->second->isVisited())
			{
				firstWalk(it->second, distance);
				defaultAncestor = apportion(it->second, defaultAncestor, distance);
			}
		}

		executeShifts(state);

		float midPoint = 0;
		float xStart = 0;
		float xEnd = 0;

		trans = state->getTransTable();

		for(transIter it = trans.begin(); it != trans.end(); ++it)
		{
			if(it == trans.begin())
			{
				xStart = it->second->getXPos();
			}
			xEnd = it->second->getXPos();
		}
		midPoint = (xStart + xEnd) / 2;

		//State* ell = state->getTransTable().begin()->second;
		//arr = state->getTransTable().end()->second;
		State* leftBro = state->leftBrother();

		if(leftBro != nullptr)
		{
			state->setXPos(leftBro->getXPos() + distance);
			state->setMod(state->getXPos() - midPoint);
		}
		else
		{
			state->setXPos(midPoint);
		}
	}
	return state;
}

State* DFA::apportion(State* state, State* defaultAncestor, float distance)
{
	State* leftBro = state->leftBrother();

	if(state->getName() == "State3")
	{
		int a = 0;
	}

	if(leftBro != nullptr)
	{
		//in buchheim notation:
        //i == inner; o == outer; r == right; l == left; r = +; l = -

		State* vir = state;
		State* vor = state;
		State* vil = leftBro;
		State* vol = state->getLeftMostSib();

		float sir = vir->getMod();
		float sor = vor->getMod();
		float sil = vil->getMod();

		float sol = state->getMod();
		if(vol != nullptr)
		{
			sol = vol->getMod();
		}

		while(vil->right() != nullptr && vir->left() != nullptr)
		{
			vil = vil->right();
			vir = vir->left();
			
			if(vol->left() != nullptr)
			{
				vol = vol->left();
			}

			if(vor->right() != nullptr)
			{
				vor = vor->right();
				vor->setAncestor(state);
			}

			float shift = (vil->getXPos() + sil) - (vir->getXPos() + sir) + distance;

			if(shift > 0)
			{
				moveSubTree(ancestor(vil, state, defaultAncestor), state, shift/5);
				sir += shift;
				sor += shift;
			}

			sil += vil->getMod();
			sir += vir->getMod();
			
			sol += vol->getMod();
			sor += vor->getMod();
		}

		if(vil->right() != nullptr && vol != nullptr && vor->right() == nullptr)
		{
			vor->setThread(vil->right());
			vor->setMod(vor->getMod() + (sil - sor));
		}
		else
		{
			if(vir->left() != nullptr && vol != nullptr && vol->left() == nullptr)
			{
				vol->setThread(vir->left());
				vol->setMod(vol->getMod() + (sir - sol));
			}
			defaultAncestor = state;
		}
	}

	return defaultAncestor;
}

void DFA::moveSubTree(State* ancestor, State* state, float shift)
{
	//wl = ancestor wr = state;

	int subtrees = state->getGroupNum() - ancestor->getGroupNum();

	if(subtrees != 0)
	{
		state->setChange(state->getChange() - (shift / subtrees));
	}
	
	state->setShift(state->getShift() + shift);
	
	if(subtrees != 0)
	{
		ancestor->setChange(ancestor->getChange() + (shift / subtrees));
	}

	state->setXPos(state->getXPos() + shift);
	state->setMod(state->getMod() + shift);
}

void DFA::executeShifts(State* state)
{
	float shift = 0;
	float change = 0;

	transTable trans = state->getTransTable();

	for(transIter it = trans.begin(); it != trans.end(); ++it)
	{
		it->second->setXPos(it->second->getXPos() + shift);
		it->second->setMod(it->second->getMod() + shift);
		change = it->second->getChange();
		shift += it->second->getShift() + change;
	}
}

State* DFA::ancestor(State* vil, State* state,  State* defaultAncestor)
{
	transTable trans = state->getParent()->getTransTable();
	
	for(transIter it = trans.begin(); it != trans.end(); ++it)
	{
		if(vil->getAncestor() == it->second)
		{
			return vil->getAncestor();
		}
	}
	return defaultAncestor;
}

float DFA::secondWalk(State* state, float m, float min)
{
	state->setVisited(true);
	state->setXPos(state->getXPos() + m);

	if(min == NULL || state->getXPos() < min)
	{
		min = state->getXPos();
	}

	transTable trans = state->getTransTable();

	for(transIter it = trans.begin(); it != trans.end(); ++it)
	{
		if(!it->second->isVisited())
		{
			min = secondWalk(it->second, m + state->getMod(), min);
		}
	}

	return min;
}
	
void DFA::thirdWalk(State* state, float min)
{
	state->setVisited(true);
	state->setXPos(state->getXPos() + min);

	transTable trans = state->getTransTable();

	for(transIter it = trans.begin(); it != trans.end(); ++it)
	{
		if(!it->second->isVisited())
		{
			thirdWalk(it->second, min);
		}
	}
}