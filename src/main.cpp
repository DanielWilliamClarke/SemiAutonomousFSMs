//////////////////////////////////////////////////////
// author Daniel Clarke 2013/14

#include <math.h>
#include <algorithm>

#include "OPCClient.h"
#include "DFABuilder.h"
#include "ControllerUI.h"

// prototypes
void outputResults(DFA*);
void printTrans(Trans_Input input);
void printOutputs(State_Output* output);

int main(int argc, char** argv)
{
	srand((unsigned int)time(NULL));

	std::string toParse;
	std::string OPCServer = "Kepware.KEPServerEX.V5";
	bool manTranOccured = false;
	float distanceX = 0;
	float distanceY = 0;
	sf::Vector2f* onFirstClickVec = nullptr;
	float zoomFactor = 1;

	sf::Font font; //draw text
	if(!font.loadFromFile("C:/Windows/Fonts/segoeui.ttf"))
	{
		std::cout << "Could not Load Font File" << std::endl;
		std::cin.get();
		return 0;
	}
	
	while(true)
	{
		try
		{
			std::cin.get();
			system("CLS");
			std::cout << "Dyanamic Semi-Autonomous Extended Finite State Machine Control System" << std::endl << std::endl;
			std::cout << "Please Enter Your Finite State Machine Configuration:" << std::endl << std::endl;
			
			// User input
			getline(cin, toParse);

			// comment the line above , and uncomment a single line below to use a pre made regular expression
			//toParse = "([@/State1/MatlabOutputsB>1{D/MatlabOutputsA=0,VisualStudioOutputsB=0,VisualStudioOutputsA=0,MatlabOutputsB=0}{E/MatlabOutputsB=0}]|[@/State1/MatlabOutputsB>2{D/MatlabOutputsA=0,VisualStudioOutputsB=0,VisualStudioOutputsA=0,MatlabOutputsB=0}{E/MatlabOutputsB=0}])*";
			//toParse = "([@/State1/MatlabOutputsB>0&&MatlabOutputsB>3&&MatlabOutputsB>9&&MatlabOutputsB>8{D/MatlabOutputsA=0,VisualStudioOutputsB=0,VisualStudioOutputsA=0,MatlabOutputsB=0}{E/MatlabOutputsB=0}]([@/State2/MatlabOutputsB>3{SD/MatlabOutputsB=0}{D/MatlabOutputsB=0,MatlabOutputsB=0,MatlabOutputsB=0,MatlabOutputsB=0,MatlabOutputsB=0}]|[@/State3/MatlabOutputsB<4&&MatlabOutputsB>=2{D/MatlabOutputsB=0}])*([@/State10/MatlabOutputsB>7646&&MatlabOutputsB<246{D/MatlabOutputsB=0}][@/State4/MatlabOutputsB>35636&&MatlabOutputsB<246{D/MatlabOutputsB=0}]|[@/State5/MatlabOutputsB>0&&MatlabOutputsB<1{D/MatlabOutputsB=0}])([@/State6/MatlabOutputsB>19{D/MatlabOutputsB=7}]|[@/State7/MatlabOutputsB<8{D/MatlabOutputsB=400}])*[@/State8/MatlabOutputsB>458&&MatlabOutputsB<246{D/MatlabOutputsB=0}][@/State9/MatlabOutputsB>576435&&MatlabOutputsB<246{D/MatlabOutputsB=0}])";
			//toParse = "[@/State0/MatlabOutputsB>8{D/MatlabOutputsA=0,VisualStudioOutputsB=0,VisualStudioOutputsA=0,MatlabOutputsB=0}{E/MatlabOutputsB=0}]([@/State2/MatlabOutputsB>1{D/MatlabOutputsA=0,VisualStudioOutputsB=0,VisualStudioOutputsA=0,MatlabOutputsB=1}{E/MatlabOutputsB=0}]|[@/State1/MatlabOutputsB>2{D/MatlabOutputsA=0,VisualStudioOutputsB=0,VisualStudioOutputsA=0,MatlabOutputsB=400}{E/MatlabOutputsB=0}])*";
			
			// commnet the lines above and uncomment the 2 lines below to use this regular expression
			//toParse = "([@/State1/MatlabOutputsB<60&&MatlabOutputsB>3{SD/MatlabOutputsB=0.054}{D/MatlabOutputsA=0,VisualStudioOutputsB=40,VisualStudioOutputsA=40,MatlabOutputsB=50}{E/MatlabOutputsB=1}]([@/State2/VisualStudioOutputsB>3{D/MatlabOutputsB=20,MatlabOutputsA=0,VisualStudioOutputsB=40,VisualStudioOutputsA=40}]|[@/State3/VisualStudioOutputsA<=40&&MatlabOutputsA!=0&&MatlabOutputsB>=2{D/MatlabOutputsB=56, VisualStudioOutputsB=0.1}])*([@/State10/VisualStudioOutputsB<1{D/MatlabOutputsB=0, MatlabOutputsA=1}][@/State4/MatlabOutputsB>35||MatlabOutputsA>0.5{D/MatlabOutputsA=0,VisualStudioOutputsB=56.5,VisualStudioOutputsA=0.4302}]|[@/State5/VisualStudioOutputsB>56.2&&MatlabOutputsB<1{D/MatlabOutputsB=12,MatlabOutputsA=70}])([@/State6/MatlabOutputsB>19{D/MatlabOutputsB=7}]|[@/State7/MatlabOutputsB<8{D/MatlabOutputsB=400}])*[@/State8/MatlabOutputsB>63&&MatlabOutputsB<246{D/MatlabOutputsB=0}][@/State9/MatlabOutputsB>45&&MatlabOutputsB<246{D/MatlabOutputsB=0}])";
			//toParse += toParse + "*";

			std::cout << "Building DFA from Parse String:" << std::endl << std::endl << toParse  << std::endl << std::endl;
			//remove any whitespace
			toParse.erase(remove_if(toParse.begin(), toParse.end(), isspace), toParse.end()); 

			// Construction Phase
			auto start = std::chrono::steady_clock::now();
			DFABuilder dfactory(toParse);
			DFA* dfa = dfactory.getDFA();
			outputResults(dfa);

			// Main Control Loop setup
			State* currentState = dfa->getInitialState().at(0);
			State* previousState = nullptr;
			std::vector<int> manualStates = currentState->getManualStateIndexes();

			OPCClientInput* opcClientI = nullptr;
			OPCClientOutput* opcClientO = nullptr;
			try
			{
				std::cout << "Attempting Connection to OPC Server on: " << OPCServer << " ... " << std::endl;
				opcClientI = new OPCClientInput(OPCServer, dfa->getInputChannels());
				opcClientO = new OPCClientOutput(OPCServer, dfa->getOutputChannels());
			}
			catch(SSOPC_Exception &ex)
			{
				std::cout << ex.ErrDesc() << std::endl << std::endl;
				// to get the main loop to tick back to user input again
				continue;
			}

			std::cout << "OPC Client on: " << OPCServer << " Connected: " << std::endl;
			std::cout << opcClientI->getChannelSet().size() << " Input Channels, " << std::endl;
			std::cout << opcClientO->getChannelSet().size() << " Output Channels " << std::endl;
			std::cout << "Generating User Interface ... " << std::endl << std::endl;
			std::cout << "--------------------------------------------------------------------" << std::endl << std::endl;
			std::cout << "Reading Input Values ... " << std::endl << std::endl;

			// Generate FSM Representation
			sf::RenderWindow window(sf::VideoMode(SCREENWIDTH, SCREENHEIGHT), "Semi-Autonomous Finite State Controller");
			sf::View camera(sf::FloatRect(0, 0, SCREENWIDTH, SCREENHEIGHT));
			camera.setCenter(SCREENWIDTH/2, SCREENHEIGHT/2);

			sf::Vertex Background[] =
			{
				//top left
				sf::Vertex(sf::Vector2f(0, 0), sf::Color(13, 24, 59, 255)),
				//bottom left
				sf::Vertex(sf::Vector2f(SCREENWIDTH, 0), sf::Color(31, 78, 121, 255) /*sf::Color(13, 24, 59, 255)*/),
				//bottom right
				sf::Vertex(sf::Vector2f(SCREENWIDTH, SCREENHEIGHT), sf::Color(31, 78, 121, 255)),
				//top right
				sf::Vertex(sf::Vector2f(0, SCREENHEIGHT), sf::Color(31, 78, 121, 255) /*sf::Color(13, 24, 59, 255)*/)
			};

			ControllerUI* ui = new ControllerUI(dfa->getStateVec(), &window, &font);
			RealTimeDataUI* inputUI = new RealTimeDataUI(&window, "Real Time Input Data", dfa->getInputChannels(), 0, &font);
			RealTimeDataUI* outputUI = new RealTimeDataUI(&window, "Control Commands", dfa->getOutputChannels(), SCREENWIDTH-200, &font);

			auto end = std::chrono::steady_clock::now();
			std::cout << "FSM Construction with " << dfa->numberOfCondtions + 1 << " States, OPC Deployment, and User Interface Generation Completed in: " << std::endl << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " seconds" << std::endl << std::endl;

			try
			{
				// Main Control Loop
				while(window.isOpen())
				{	
					sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
					sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);

					previousState = currentState;

					sf::Event event;
					while (window.pollEvent(event))
					{
						if (event.type == sf::Event::Closed)
						{
							window.close();
						}

						if(event.type == sf::Event::MouseButtonReleased)
						{
							if(event.mouseButton.button == sf::Mouse::Left)
							{
								State* tempState = ui->manualSelectState(worldPos, manualStates);
								if(tempState != nullptr)
								{
									currentState = tempState;
									manTranOccured = true;
								}
							}
						}
					}

					// reset initial state
					if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
					{
						currentState = dfa->getInitialState().at(0); 
						manTranOccured = true;
					}

					// screen dragging
					if(sf::Mouse::isButtonPressed(sf::Mouse::Right))
					{
						//for first click
						if(onFirstClickVec == nullptr)
						{
							onFirstClickVec = new sf::Vector2f(worldPos);
						}

						distanceX = worldPos.x - onFirstClickVec->x;
						distanceY = worldPos.y - onFirstClickVec->y;

						camera.move(-distanceX, -distanceY);

						delete onFirstClickVec;
						onFirstClickVec = new sf::Vector2f(worldPos.x-distanceX, worldPos.y-distanceY);
					}
					else
					{
						if(onFirstClickVec != nullptr)
						{
							delete onFirstClickVec;
							onFirstClickVec = nullptr;
						}
					}

					// screen zooming
					if(sf::Keyboard::isKeyPressed(sf::Keyboard::Add))
					{
						camera.zoom(zoomFactor - 0.05f);
					}

					if(sf::Keyboard::isKeyPressed(sf::Keyboard::Subtract))
					{
						camera.zoom(zoomFactor + 0.05f);
					}

					start = std::chrono::steady_clock::now();
					opcClientI->readChannels();
					end = std::chrono::steady_clock::now();
					//std::cout << "Reading " << opcClientO->getChannelSet().size() << " Data Channels Took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " milliseconds" << std::endl << std::endl;

					if(!manTranOccured)
					{
						currentState = currentState->determineNextTransition(opcClientI->getChannelSet());  
					}
					manTranOccured = false;

					if(previousState == currentState)
					{
						//output on during
						if(currentState->getOnDuringOutputs() != nullptr)
						{
							opcClientO->spliceStateOutput(currentState->getOnDuringOutputs());
						}
					}
					else
					{
						manualStates = currentState->getManualStateIndexes();
						//output on entry
						std::cout << "Transition from State: " << previousState->getName() << " To State: " << currentState->getName() << std::endl;
						if(currentState->getOnEntryOutputs() != nullptr)
						{
							opcClientO->spliceStateOutput(currentState->getOnEntryOutputs());
						}
					}	
				
					start = std::chrono::steady_clock::now();
					opcClientO->writeChannels();
					end = std::chrono::steady_clock::now();
					//std::cout << "Writing to " << opcClientI->getChannelSet().size() << " Data Channels Took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " milliseconds" << std::endl << std::endl;
					
					// refresh screen
					window.clear();

					//draw background
					window.setView(window.getDefaultView());
					window.draw(Background, 20, sf::Quads);
					window.setView(camera);

					// redrawn FSM
					ui->redrawAll(currentState->getIndex(), manualStates);
					window.setView(window.getDefaultView());

					// update real time data
					inputUI->redraw(opcClientI->getChannelSet()); // dfa->getInputChannels());
					outputUI->redraw(opcClientO->getChannelSet()); // dfa->getOutputChannels());
					window.setView(camera);

					// check for state hover event and has to be drawn on top of all else
					ui->stateCollision(worldPos, &font);
					window.display();
					Sleep(50);
				}
			}
			catch(SSOPC_Exception &ex)
			{
				std::cout << ex.ErrDesc() << std::endl << std::endl;
				std::cout << "RESTARTING SYSTEM!!";
						window.close();
			}
			
			// delete allocated memory
			delete dfa;
			delete opcClientI;
			delete opcClientO;
			delete ui;
			delete inputUI;
			delete outputUI;
		}
		catch(std::exception& ex)
		{
			std::cout << ex.what() << std::endl << std::endl;
		}
	}

	std::cin.get();

    return 0;
}

void outputResults(DFA* dfa)
{
	std::cout << "--------------------------------------------------------------------" << std::endl << std::endl;
	std::cout << "Start (Initial) State(s): " << " ---------------------" << std::endl;
	stateVec vec = dfa->getInitialState();
	for(StateIter it = vec.begin(); it != vec.end(); ++it)
	{
		std::cout << "State: " << (*it)->getIndex() << std::endl;
	}
	std::cout << std::endl;

	std::cout << "Final (Accepting) State(s): " << " -------------------" << std::endl;
	vec = dfa->getFinalState();
	for(StateIter it = vec.begin(); it != vec.end(); ++it)
	{
		std::cout << "State: " << (*it)->getIndex() << std::endl;
	}
	std::cout << std::endl;

	std::cout << "--------------------------------------------------------------------" << std::endl << std::endl;
	std::cout << "DFA Transition table ---------------------------" << std::endl << std::endl;
	std::cout << "--------------------------------------------------------------------" << std::endl << std::endl;
	vec = dfa->getStateVec();
	for(StateIter it = vec.begin(); it != vec.end(); ++it)
	{
		transTable trans = (*it)->getTransTable();

		std::cout << "State: " << (*it)->getIndex() << " Name: " << (*it)->getName() << " at Depth " << (*it)->getDepth();

		if((*it)->getParent() != nullptr)
		{
			std::cout << " Parent State is State: " << (*it)->getParent()->getIndex() << std::endl;
		}
		else
		{
			std::cout << " is the Root " << std::endl;
		}

		if((*it)->isSelectable())
		{
			std::cout <<  " Manually Selectable ";
		}
		else
		{
			std::cout <<  " not Manually Selectable ";
		}
			
		std::cout<< " ------------------" << std::endl << std::endl;

		std::cout << "OnEntry: " << " ------------------" << std::endl;

		if((*it)->getOnEntryOutputs() != nullptr)
		{
			printOutputs((*it)->getOnEntryOutputs());
		}

		std::cout << "During: " << " ------------------" << std::endl;

		if((*it)->getOnDuringOutputs() != nullptr)
		{
			printOutputs((*it)->getOnDuringOutputs());
		}

		std::cout << std::endl;

		for(transIter it1 = trans.begin(); it1 != trans.end(); ++it1)
		{
			std::cout << "Transition Where Channel: ";
			printTrans(it1->first);
			std::cout << " To State " << (it1->second)->getIndex() << std::endl;
		}

		if(trans.size() == 0)
		{
			std::cout << "No Transitions" << std::endl;
		}

		std::cout << std::endl;
		std::cout << "--------------------------------------------------------------------" << std::endl << std::endl;
	}
}

//recursive function just to print out the chained transition conditions
void printTrans(Trans_Input input)
{
	std::cout << input.channel << input.opcode << input.value;
	
	if(input.nextCondtion != nullptr)
	{
		std::cout << " " << input.logicOpCode << " ";
		printTrans(*input.nextCondtion);
	}
}

void printOutputs(State_Output* output)
{
	std::cout << "On Channel: " << output->channel << " Output Value: " << output->value << std::endl;

	if(output->nextOutput != nullptr)
	{
		printOutputs(output->nextOutput);
	}
}
