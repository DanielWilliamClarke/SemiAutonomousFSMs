//////////////////////////////////////////////////////
// author Daniel Clarke 2013/14 
// based on code by Eli Bendersky (spur4444@yahoo.com)

#include "regex_parse.h"

// helper class to iterate over the input string

scanner& my_scanner(void)
{
	try
	{
		static scanner my_scan;
		return my_scan;
	}
	catch(std::exception& ex)
	{
		throw ex;
	}
}

void scanner::init(std::string toParse)
{
    data = preprocess(toParse);
	next = 0;
}

char scanner::peek(void)
{
	return (next < data.size()) ? data[next] : 0;
}

char scanner::pop(void)
{
	char currentItem = peek();

	if (next < data.size())
	{
		++next;
	}

	return currentItem;
}

unsigned scanner::get_pos(void)
{
    return next;
}

// Generates concatenation chars ('.') where appropriate

std::string scanner::preprocess(std::string in)
{
    std::string out = "";

	bool insideExpression = false; 

    auto currentItem = in.begin();
	auto nextItem = currentItem + 1;

    // in this loop c is the current char of in, up is the next one 
    for (; nextItem != in.end(); ++currentItem, ++nextItem)
    {
		out.push_back(*currentItem);

		if(*currentItem == '[')
			insideExpression = true;
		if(*currentItem == ']')
			insideExpression = false;

		if ((isalnum(*currentItem) || *currentItem == ')' || *currentItem == '*' || *currentItem == '+'|| *currentItem == ']') &&
			(*nextItem != ')' && *nextItem != '|' && *nextItem != '*' && *nextItem != '+'))
		{
			if(!insideExpression)
				out.push_back('.');
		}
    }

    // don't forget the last char ...
    if (currentItem != in.end())
	{
		out.push_back(*currentItem);
	}

    return out;
}

// Regex Parser Implementation

Regex_parse::Regex_parse(std::string toParse)
{
	if(toParse.empty())
	{
		throw std::out_of_range("Parse String is Empty");
	}

	my_scanner().init(toParse);

	if (my_scanner().peek() == 0)
    {
		// throw excpetion
		std::string str;
		str.append("Parse error: unexpected char ").append(std::to_string(my_scanner().get_pos())).append(" at #").append(std::to_string(my_scanner().get_pos()));
		throw std::length_error(str);
	}
	// make the tree
	numberOfCondtions = 0;
	tree = expr();
}

parse_node* Regex_parse::getTree()
{
	return tree;
}

void Regex_parse::print_tree(parse_node* node, unsigned offset)
{
    if (!node)
	{
		return;
	}

    for (unsigned i = 0; i < offset; ++i)
	{
		std::cout << " ";
	}

    switch (node->type)
    {
	case BASIC:
	    //std::cout << node->channel;
	    break;
	case ALTER:
	    std::cout << '|';
	    break;
	case CONCAT:
	    std::cout << '.';
	    break;
	case STAR:
	    std::cout << '*';
	    break;
	default:
	    assert(0);
    }

    std::cout << std::endl;

    print_tree(node->left, offset + 4);
    print_tree(node->right, offset + 4);
}

// char   ::= alphanumeric character
parse_node* Regex_parse::chr(void)
{
	//he'll we'll construct the input object
	// then attach it to the state object
    //char data = my_scanner().peek();

    if (my_scanner().peek() == '[')
    {
		//recurse to get the channel name
		++numberOfCondtions;
		return new parse_node(BASIC, makeInputOutput(new Regex_Parse_Data(nullptr), ""), nullptr, nullptr);
    }
	else
	{
		//if its not alphanumeric or an opcode fragment throw exception something doesnt belong here
		throw std::invalid_argument("Parse error: expected a starting '['");   
	}

	std::string str;
	str.append("Parse error: unexpected char ").append(std::to_string(my_scanner().get_pos())).append(" at #").append(std::to_string(my_scanner().get_pos()));
	throw std::length_error(str);
}

Regex_Parse_Data* Regex_parse::makeInputOutput(Regex_Parse_Data* nodeData, std::string outputType)
{
	try
	{
		char data = my_scanner().peek();
		if(data == '[')
		{
			my_scanner().pop();
			data = my_scanner().peek();
			if(data == '@')
			{
				nodeData->selectable = true;
				my_scanner().pop();
				data = my_scanner().peek();
			}
			else
			{
				nodeData->selectable = false;
			}

			if(data == '/')
			{
				my_scanner().pop();
				nodeData->stateName = makeStateName("");
			}
			else
			{
				//if its not alphanumeric or an opcode fragment throw exception something doesnt belong here
				throw std::invalid_argument("Parse error: State Name Expected ");   
			}

			nodeData->inputCondition = makeInputChan(new Trans_Input());
		}

		data = my_scanner().peek();

		if(data == '{' || data == 'D' || data == 'E' || data == 'S' || data == '/')
		{
			if(data == '{')
			{
				my_scanner().pop();
			}
			//get characters
			outputType += my_scanner().pop();
			return makeInputOutput(nodeData, outputType);
		}
		else if (data == ']')
		{
			my_scanner().pop();
			return nodeData;
		}
		else if (outputType.size() > 0)
		{
			if(outputType == "D/")
			{
				//recurse to get opcode
				nodeData->stateOutputDuring = makeOutputChan(new State_Output());
			}
			else if (outputType == "E/")
			{
				nodeData->stateOutputEntry = makeOutputChan(new State_Output());
			}
			else if(outputType == "SD/")
			{
				nodeData->StartStateOutputDuring = makeOutputChan(new State_Output());
			}
			else if(outputType == "SE/")
			{
				nodeData->StartStateOutputEntry = makeOutputChan(new State_Output());
			}
			else
			{
				//if its not alphanumeric or an opcode fragment throw exception something doesnt belong here
				throw std::invalid_argument("Parse error: expected 'D/', 'E/', 'SD/' or 'SE/");   
			}
			return makeInputOutput(nodeData, "");
		}
		else
		{
			//if its not alphanumeric or an opcode fragment throw exception something doesnt belong here
			throw std::invalid_argument("Parse error: Missing ']'");   
		}
	}
	catch(std::exception& ex)
	{
		throw ex;
	}
}

std::string Regex_parse::makeStateName(std::string input)
{
	try
	{
		char data = my_scanner().peek();
		if(isalnum(data) || data == '_' || data == '-')
		{
			//get characters
			input += my_scanner().pop();
			return makeStateName(input);
		}
		else if (data == '/')
		{
			my_scanner().pop();
			return input;
		}
		else
		{
			input += my_scanner().pop();
			//if its not alphanumeric or an opcode fragment throw exception something doesnt belong here
			throw std::invalid_argument("Parse error: State Name " + input + " is Invalid");   
		}
	}
	catch(std::exception& ex)
	{
		throw ex;
	}
}

Trans_Input* Regex_parse::makeInputChan(Trans_Input* input)
{
	try 
	{
		//construct new complex type 
		char data = my_scanner().peek();
		//need to be the start of the operator, could use isalnum but then could have any old operator going in
		if(isalnum(data) || data == '_' || data == '-')
		{
			//get characters
			input->channel += my_scanner().pop();
			return makeInputChan(input);
		}
		else if(data == '=' || data == '!' || data == '>'|| data == '<')
		{
			if(input->channel.size() == 0)
			{
				throw std::invalid_argument("Parse error: Channel Name is empty");   
			}
			//recurse to get opcode
			addToInputChannels(input->channel);
			return makeInputOp(input);
		}
		else
		{
			input->channel += my_scanner().pop();
			//if its not alphanumeric or an opcode fragment throw exception something doesnt belong here
			throw std::invalid_argument("Parse error: Channel Name " + input->channel + "is Invalid");   
		}
	}
	catch(std::exception& ex)
	{
		throw ex;
	}
}

Trans_Input* Regex_parse::makeInputOp(Trans_Input* input)
{
	try 
	{
		char data = my_scanner().peek();
		if(data == '=' || data == '!' || data == '>'|| data == '<')
		{
			input->opcode += my_scanner().pop();
			return makeInputOp(input);
		}
		else if(isalnum(data) || data == '-')
		{ 
			if(input->opcode == "==" || input->opcode == "!=" || input->opcode == ">" || input->opcode == "<" 
				||  input->opcode == ">=" || input->opcode == "<=")    
			{
				//we have a number and a valid operator
				return makeInputVal(input, "");
			}
			else
			{
				throw std::invalid_argument("Parse error: opCode " + input->opcode + " is invalid"); 
			}
		}
		else
		{
			input->opcode += my_scanner().pop();
			//if its not alphanumeric or an opcode fragment throw exception something doesnt belong here
			throw std::invalid_argument("Parse error: Operator " + input->opcode + " is Invalid");   
		}
	}
	catch(std::exception& ex)
	{
		throw ex;
	}
}

Trans_Input* Regex_parse::makeInputVal(Trans_Input* input, std::string inputVal)
{
	//construct new complex type 
	try
	{
		double testValue;
		char data = my_scanner().peek();
		//the period is there for doubles. the minus symbol is there for negative values
		if(isalnum(data) || data == '.' || data == '-')
		{
			inputVal += my_scanner().pop();
			return makeInputVal(input, inputVal);
		}
		else if(data == ']' || data == '{')
		{
			std::istringstream testStream(inputVal);
			input->nextCondtion = nullptr;
			//my_scanner().pop();

			//catch if the value is a boolean value
			if (inputVal == "true")
			{
				input->value = 1;
			}
			else if(inputVal == "false")
			{
				input->value = 0;
			}
			else if (!(testStream >> testValue) || inputVal.size() == 0)
			{
				//catch if the value is a valid value, negative or positve with no alphabetical characters
				throw std::invalid_argument("Parse error: Output Value " + inputVal + " is invalid");  
			}
			else 
			{			
				//if the value can be parsed by contains alphabet characters this should clear them out
				//little bit of a hack here but..
				input->value = testValue;
			}
			return input;
		}
		else if(data == '&' || data == '|')
		{
			std::istringstream testStream(inputVal);
			input->nextCondtion = nullptr;

			//catch if the value is a boolean value
			if (inputVal == "true")
			{
				input->value = 1;
			}
			else if(inputVal == "false")
			{
				input->value = 0;
			}
			else if(inputVal.size() == 0)
			{
				throw std::invalid_argument("Parse error: Input Value is empty");  
			}
			else if (!(testStream >> testValue))
			{
				//catch if the value is a valid value, negative or positve with no alphabetical characters
				throw std::invalid_argument("Parse error: Output Value " + inputVal + " is invalid");  
			}
			else 
			{			
				//if the value can be parsed by contains alphabet characters this should clear them out
				//little bit of a hack here but..
				input->value = testValue;
			}
			return makeInputLogicOp(input);
		}
		else
		{
			inputVal += my_scanner().pop();
			std::string str = "Parse error: expected alphanumeric or missing a '] ";
			str += inputVal.c_str();
			str += " is invalid";
			throw std::invalid_argument(str);   
		}
	}
	catch(std::exception& ex)
	{
		throw ex;
	}
}

Trans_Input* Regex_parse::makeInputLogicOp(Trans_Input* input)
{

	char data = my_scanner().peek();
	if(data == '&' || data == '|')
	{
		input->logicOpCode += my_scanner().pop();
		return makeInputLogicOp(input);
	}
	else if(isalnum(data))
	{
		if(input->logicOpCode == "&&" || input->logicOpCode == "||")
		{
			//we have another input
			input->nextCondtion = makeInputChan(new Trans_Input());
			return input;
		}
		else
		{
			throw std::invalid_argument("Parse error: Logic Op " + input->logicOpCode + " is invalid");  
		}
	}
	else
	{
		input->logicOpCode += my_scanner().pop();
		throw std::invalid_argument("Parse error: expected alphanumeric " + input->logicOpCode + "is Invalid");   
	}
}

State_Output* Regex_parse::makeOutputChan(State_Output* output)
{
	try 
	{
		//construct new complex type 
		char data = my_scanner().peek();
		//need to be the start of the operator, could use isalnum but then could have any old operator going in
		if(isalnum(data) || data == '_' || data == '-')
		{
			//get characters
			output->channel += my_scanner().pop();
			return makeOutputChan(output);
		}
		else if(data == '=')
		{
			if(output->channel.size() == 0)
			{
				throw std::invalid_argument("Parse error: Channel Name is empty");   
			}
			//recurse to get opcode
			addToOutputChannels(output->channel);
			my_scanner().pop();
			return makeOutputVal(output, "");
		}
		else
		{
			//if its not alphanumeric or an opcode fragment throw exception something doesnt belong here
			throw std::invalid_argument("Parse error: expected alphanumeric or legal opCode " + output->channel + " is Invalid");   
		}
	}
	catch(std::exception& ex)
	{
		throw ex;
	}
}

State_Output* Regex_parse::makeOutputVal(State_Output* output, std::string outputVal)
{
	try
	{
		double testValue;
		char data = my_scanner().peek();
		//the period is there for doubles. the minus symbol is there for negative values
		if(isalnum(data) || data == '.' || data == '-')
		{
			outputVal += my_scanner().pop();
			return makeOutputVal(output, outputVal);
		}
		else if(data == '}')
		{
			std::istringstream testStream(outputVal);
			output->nextOutput = nullptr;
			my_scanner().pop();

			//catch if the value is a boolean value
			if (outputVal == "true")
			{
				output->value = 1;
			}
			else if(outputVal == "false")
			{
				output->value = 0;
			}
			else if(outputVal.size() == 0)
			{
				throw std::invalid_argument("Parse error: Output Value is empty");  
			}
			else if (!(testStream >> testValue))
			{
				//catch if the value is a valid value, negative or positve with no alphabetical characters
				throw std::invalid_argument("Parse error: Output Value " + outputVal + " is invalid");  
			}
			else 
			{			
				//if the value can be parsed by contains alphabet characters this should clear them out
				//little bit of a hack here but..
				output->value = testValue;
			}
			return output;
		}
		else if (data == ',')
		{
			std::istringstream testStream(outputVal);
			output->nextOutput = nullptr;
			my_scanner().pop();

			//catch if the value is a boolean value
			if (outputVal == "true")
			{
				output->value = 1;
			}
			else if(outputVal == "false")
			{
				output->value = 0;
			}
			else if (!(testStream >> testValue))
			{
				//catch if the value is a valid value, negative or positve with no alphabetical characters
				throw std::invalid_argument("Parse error: Output Value " + outputVal + " is invalid");  
			}
			else 
			{			
				//if the value can be parsed by contains alphabet characters this should clear them out
				//little bit of a hack here but..
				output->value = testValue;
			}
			output->nextOutput = makeOutputChan(new State_Output());
			return output;
		}
		else
		{
			outputVal += my_scanner().pop();
			throw std::invalid_argument("Parse error: expected alphanumeric " + outputVal + " is invalid - perhaps missings a } or ,");   
		}
	}
	catch(std::exception& ex)
	{
		throw ex;
	}
}

// atom   ::= chr 
//          | '(' expr ')'
parse_node* Regex_parse::atom(void)
{
	try{
		parse_node* atom_node;
    
		if (my_scanner().peek() == '(')
		{
			my_scanner().pop();
			atom_node = expr();
	
			if (my_scanner().pop() != ')')
			{
				// throw exception
				throw std::invalid_argument("Parse error: expected ')'");   
			}
		}
		else
		{
			atom_node = chr();
		}

		return atom_node;
	}
	catch(std::exception& ex)
	{
		throw ex;
	}
}

// rep    ::= atom '*'
//          | atom
parse_node* Regex_parse::rep(void)
{
	try
	{
		parse_node* atom_node = atom();

		if (my_scanner().peek() == '*')
		{
			// zero or more occurances
			my_scanner().pop();
			parse_node* rep_node = new parse_node(STAR, nullptr, atom_node, 0);
			return rep_node;
		}
		else if(my_scanner().peek() == '+')
		{
			// one or more occurances
			my_scanner().pop();
			parse_node* rep_node = new parse_node(PLUS, nullptr, atom_node, 0);
			return rep_node;
		}
		else
		{
			return atom_node;
		}
	}
	catch(std::exception& ex)
	{
		throw ex;
	}
}

// concat ::= rep . concat
//          | rep
parse_node* Regex_parse::concat(void)
{
	try
	{
		parse_node* left = rep();
		char data = my_scanner().peek();
		if (data == '.')
		{
			my_scanner().pop();
			parse_node* right = concat();
			parse_node* concat_node = new parse_node(CONCAT, nullptr, left, right);
			return concat_node;
		}
		else
		{
			return left;
		}
	}
	catch(std::exception& ex)
	{
		throw ex;
	}
}

// expr   ::= concat '|' expr
//          | concat
parse_node* Regex_parse::expr(void)
{
	try
	{
		parse_node* left = concat();
	
		if (my_scanner().peek() == '|')
		{
			my_scanner().pop();
			parse_node* right = expr();
			parse_node* expr_node = new parse_node(ALTER, nullptr, left, right);
			return expr_node;
		}
		else
		{
			return left;
		}
	}
	catch(std::exception& ex)
	{
		throw ex;
	}
}

void Regex_parse::addToInputChannels(std::string channelName)
{
	inputChannels[channelName] = 0;
}

void Regex_parse::addToOutputChannels(std::string channelName)
{
	outputChannels[channelName] = 0;
}

std::map<std::string, double> Regex_parse::getInputChannels()
{
	return inputChannels;
}

std::map<std::string, double> Regex_parse::getOutputChannels()
{
	return outputChannels;
}