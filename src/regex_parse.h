//////////////////////////////////////////////////////
// author Daniel Clarke 2013/14

#ifndef REGEX_PARSE_H
#define REGEX_PARSE_H

#include <map>
#include <string>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <exception>
#include "Trans_Input.h"

// enums for the parse tree node type
typedef enum types {BASIC, ALTER, CONCAT, STAR, PLUS} node_type;

class scanner
{
public:

	void init(std::string);
	char peek(void);
	char pop(void);
	unsigned get_pos(void);
	friend scanner& my_scanner(void);
private:
	
	std::string preprocess(std::string);
	std::string data;
	unsigned next;
};

// Parse node
//
struct parse_node
{

	parse_node(node_type type_, Regex_Parse_Data* data_, parse_node* left_, parse_node* right_)
	    : type(type_), data(data_), left(left_), right(right_) {}
	node_type type;
	//char will become complex struct
	Regex_Parse_Data* data;
	parse_node* left;
	parse_node* right;	
};

class Regex_parse
{
public:

	// constructor builds the parse tree
	Regex_parse(std::string);
	// only need to grab the tree afterwards
	parse_node* getTree();
	std::map<std::string, double> getInputChannels();
	std::map<std::string, double> getOutputChannels();
	void print_tree(parse_node*, unsigned);
	int numberOfCondtions;
private:

	//standard regex constuction syntax functions
	parse_node* expr(void);
	parse_node* concat(void);
	parse_node* rep(void);
	parse_node* atom(void);
	parse_node* chr(void);

	// complex transition object syntax functions
	Regex_Parse_Data* makeInputOutput(Regex_Parse_Data* nodeData, std::string outputType);
	std::string makeStateName(std::string);

	Trans_Input* makeInputChan(Trans_Input*);
	Trans_Input* makeInputOp(Trans_Input*);
	Trans_Input* makeInputVal(Trans_Input*, std::string);
	Trans_Input* makeInputLogicOp(Trans_Input*);

	State_Output* makeOutputChan(State_Output*);
	State_Output* makeOutputVal(State_Output*, std::string);

	parse_node* tree;

	// opc input and output channels
	std::map<std::string, double> inputChannels;
	std::map<std::string, double> outputChannels;
	
	void addToInputChannels(std::string);
	void addToOutputChannels(std::string);
};

#endif // REGEX_PARSE_H