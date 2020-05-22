//////////////////////////////////////////////////////
// author Daniel Clarke 2013/14

#ifndef DFABUILDER_H
#define DFABUILDER_H

#include "regex_parse.h"
#include "dfa.h"

typedef std::vector<State*>::iterator StateIter;

class DFABuilder
{
public:

	DFABuilder(std::string);
	DFA* getDFA();
private:

	DFA* dfa;
	DFA* tree_to_dfa(parse_node*);
	DFA* build_DFA_basic(Regex_Parse_Data* in);
	DFA* build_DFA_alter(DFA* DFA1, DFA* DFA2);
	DFA* build_DFA_concat(DFA* DFA1, DFA* DFA2);
	DFA* build_DFA_star(DFA* DFA);
	DFA* build_DFA_plus(DFA* DFA);
	void minimiseAndFix();
};

#endif // DFABUILDER_H

