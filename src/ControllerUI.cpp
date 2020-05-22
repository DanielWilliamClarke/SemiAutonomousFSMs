//////////////////////////////////////////////////////
// author Daniel Clarke 2013/14

#include <math.h>
#include <windows.h>
#include <sstream>
#include "ControllerUI.h"

ControllerUI::ControllerUI(stateVec states, sf::RenderWindow* win, sf::Font* f)
{
	window = win;
	
	for(StateIter it = states.begin(); it != states.end(); ++it)
	{
		if((*it)->getParent() == nullptr)
		{
			alignIncrement = ((SCREENHEIGHT/2) - (*it)->getXPos()) - 50; 
		}
	}

	for(StateIter it = states.begin(); it != states.end(); ++it)
	{
		StateUI* StateCircle = new StateUI((*it), alignIncrement, f);
		uiStates.push_back(StateCircle);
		// will need to create a vector of objects which contain a main state shape, a line for each trans, and texts
	}
}

void ControllerUI::redrawAll(int currentStateIndex, std::vector<int> manualStates)
{
	for(std::vector<StateUI*>::iterator it = uiStates.begin(); it != uiStates.end(); ++it)
	{
		(*it)->redrawTrans(window, currentStateIndex);
	}
	for(std::vector<StateUI*>::iterator it = uiStates.begin(); it != uiStates.end(); ++it)
	{
		(*it)->redrawStates(window, currentStateIndex, manualStates);
	}
}

State* ControllerUI::manualSelectState(sf::Vector2f mousePos, std::vector<int> manualStates)
{
	//get manual state hit boxes
	std::vector<StateUI*> manualStateHitBoxes;
	for(std::vector<StateUI*>::iterator it = uiStates.begin(); it != uiStates.end(); ++it)
	{
		for(std::vector<int>::iterator it1 = manualStates.begin(); it1 != manualStates.end(); ++it1)
		{
			if((*it)->getIndex() == *it1)
			{
				manualStateHitBoxes.push_back(*it);
			}
		}
	}

	//perform the collision detection
	for(std::vector<StateUI*>::iterator it = manualStateHitBoxes.begin(); it != manualStateHitBoxes.end(); ++it)
	{
		if(mousePos.x >= (*it)->getStateCircle().getPosition().x 
			&& mousePos.x <= (*it)->getStateCircle().getPosition().x
			+(*it)->getStateCircle().getLocalBounds().width
			&& mousePos.y >= (*it)->getStateCircle().getPosition().y
			&& mousePos.y <= (*it)->getStateCircle().getPosition().y
			+(*it)->getStateCircle().getLocalBounds().height)
		{
			return (*it)->getMyState();
		}
	}
	return nullptr;
}

void ControllerUI::stateCollision(sf::Vector2f mousePos, sf::Font* f)
{
	//perform the collision detection
	float height = 0;
	float width = 0;
	sf::Vector2f tempPos(mousePos);
	for(std::vector<StateUI*>::iterator it = uiStates.begin(); it != uiStates.end(); ++it)
	{
		if(mousePos.x >= (*it)->getStateCircle().getPosition().x 
			&& mousePos.x <= (*it)->getStateCircle().getPosition().x
			+(*it)->getStateCircle().getLocalBounds().width
			&& mousePos.y >= (*it)->getStateCircle().getPosition().y
			&& mousePos.y <= (*it)->getStateCircle().getPosition().y
			+(*it)->getStateCircle().getLocalBounds().height)
		{

			std::vector<sf::Text> entryOuputs;
			sf::Text outputEntryTitle;
			if((*it)->getMyState()->getOnEntryOutputs() != nullptr)
			{
				outputEntryTitle.setFont(*f);
				outputEntryTitle.setString("On Entry Outputs");
				outputEntryTitle.setCharacterSize(16);
				outputEntryTitle.setColor(sf::Color::White);
				outputEntryTitle.setPosition(tempPos.x+10, tempPos.y);
				
				float tempWidth = outputEntryTitle.getLocalBounds().left + outputEntryTitle.getLocalBounds().width;

				if(width < tempWidth)
				{
					width = tempWidth;
				}

				entryOuputs = buildOutputDisplay((*it)->getMyState()->getOnEntryOutputs(), tempPos, f);
				tempWidth = calcRectXLength(entryOuputs);

				if(width < tempWidth)
				{
					width = tempWidth;
				}

				float increment = calcYIncrement((*it)->getMyState()->getOnEntryOutputs());
				height += increment;
				tempPos.y += increment;
			}	

			std::vector<sf::Text> duringOuputs;
			sf::Text outputDuringTitle;
			if((*it)->getMyState()->getOnDuringOutputs() != nullptr)
			{
				outputDuringTitle.setFont(*f);
				outputDuringTitle.setString("On During Outputs");
				outputDuringTitle.setCharacterSize(16);
				outputDuringTitle.setColor(sf::Color::White);
				outputDuringTitle.setPosition(tempPos.x+10, tempPos.y);

				float tempWidth = outputDuringTitle.getLocalBounds().left + outputDuringTitle.getLocalBounds().width;

				if(width < tempWidth)
				{
					width = tempWidth;
				}

				duringOuputs = buildOutputDisplay((*it)->getMyState()->getOnDuringOutputs(), tempPos, f);
				tempWidth = calcRectXLength(duringOuputs);
				
				if(width < tempWidth)
				{
					width = tempWidth;
				}

				float increment2 = calcYIncrement((*it)->getMyState()->getOnDuringOutputs());
				height += increment2;
			}

			sf::RectangleShape backGroundRect;

			if((*it)->getMyState()->getOnDuringOutputs() != nullptr || (*it)->getMyState()->getOnEntryOutputs() != nullptr) 
			{
				backGroundRect.setSize(sf::Vector2f(width+20, height));
				backGroundRect.setFillColor(sf::Color(0, 0, 33, 127));
				backGroundRect.setPosition(mousePos.x, mousePos.y);
				backGroundRect.setOutlineColor(sf::Color(255, 255, 255, 63));
				backGroundRect.setOutlineThickness(2);

				window->draw(backGroundRect);
				window->draw(outputEntryTitle);
			
				for(std::vector<sf::Text>::iterator it = entryOuputs.begin(); it != entryOuputs.end(); ++it)
				{
					window->draw(*it);
				}

				window->draw(outputDuringTitle);

				for(std::vector<sf::Text>::iterator it = duringOuputs.begin(); it != duringOuputs.end(); ++it)
				{
					window->draw(*it);
				}
			}

			return;
		}	
	}
}

std::vector<sf::Text> ControllerUI::buildOutputDisplay(State_Output* output, sf::Vector2f textPos, sf::Font* f)
{
	textPos.y += 30;

	std::vector<sf::Text> textObjects;

	do
	{
		sf::Text outputParameter;
		outputParameter.setFont(*f);

		std::stringstream ss;
		ss << output->value;

		std::string outputString = output->channel + " = " + ss.str();
		outputParameter.setString(outputString);
		outputParameter.setCharacterSize(14);
		outputParameter.setColor(sf::Color::White);
		outputParameter.setPosition(textPos.x+10, textPos.y);

		textPos.y += 30;

		textObjects.push_back(outputParameter);

		output = output->nextOutput;

	} while(output != nullptr);

	return textObjects;
}

float ControllerUI::calcYIncrement(State_Output* output)
{
	float height = 40;
	do
	{
		height += 30;
		output = output->nextOutput;
	} while(output != nullptr);

	return height;
}

float ControllerUI::calcRectXLength(std::vector<sf::Text> textObjects)
{
	float xLength = 0;

	for(std::vector<sf::Text>::iterator it = textObjects.begin(); it != textObjects.end(); ++it)
	{
		if(it->getLocalBounds().left + it->getLocalBounds().width > xLength)
		{
			xLength = it->getLocalBounds().left + it->getLocalBounds().width;
		}
	}

	return xLength;
}

StateUI::StateUI(State* stateToDraw, float yAlignIncrement, sf::Font* f)
{
	myState = stateToDraw;
	float xAlignIncrement = (SCREENHEIGHT/3);
	//draw circle
	points = 50;
	index = stateToDraw->getIndex();

	state.setRadius(RADIUS);
	state.setPointCount(points);
	state.setPosition(stateToDraw->getDepth() + xAlignIncrement, stateToDraw->getXPos()+yAlignIncrement);
	state.setFillColor(sf::Color::Red);
	state.setOutlineThickness(5);
	state.setOutlineColor(sf::Color::White);

	stateNameText.setString(stateToDraw->getName());
	stateNameText.setFont(*f);
	stateNameText.setCharacterSize(15);
	stateNameText.setColor(sf::Color::White);
	sf::Vector2f position = state.getPosition();

	sf::FloatRect textRect = stateNameText.getLocalBounds();
	stateNameText.setOrigin(textRect.left + (textRect.width / 2), textRect.top + (textRect.height / 2));
	
	stateNameText.setPosition(state.getPosition().x+RADIUS, state.getPosition().y+RADIUS);

	transTable trans = stateToDraw->getTransTable();

	for(transIter it = trans.begin(); it != trans.end(); ++it)
	{
		TransitionUI* trans = new TransitionUI(stateToDraw, it->second, it->first, yAlignIncrement, xAlignIncrement, f);
		transitions.push_back(trans);
	}
}

void StateUI::redrawTrans(sf::RenderWindow* window, int currentStateIndex)
{
	////draw trans first
	for(std::vector<TransitionUI*>::iterator it = transitions.begin(); it != transitions.end(); ++it)
	{
		(*it)->redraw(window, myState->getIndex(), currentStateIndex);
	}
}

void StateUI::redrawStates(sf::RenderWindow* window, int currentStateIndex, std::vector<int> manualStates)
{
	//determine circle colour
	if(index == currentStateIndex)
	{
		state.setFillColor(sf::Color(51, 153, 51));
	}
	else
	{
		state.setFillColor(sf::Color::Red);
	}

	for(std::vector<int>::iterator it = manualStates.begin(); it != manualStates.end(); ++ it)
	{
		if(index == *it)
		{
			state.setFillColor(sf::Color(255, 102, 00));
		}
	}
	
	window->draw(state);
	//then text
	
	window->draw(stateNameText);
}

sf::CircleShape StateUI::getStateCircle()
{
	return state;
}

State* StateUI::getMyState()
{
	return myState;
}

int StateUI::getIndex()
{
	return index;
}

TransitionUI::TransitionUI(State* startPoint, State* endPoint, Trans_Input condition, float yAlignIncrement, float xAlignIncrement, sf::Font* f)
{

	int midPointLeft = 0;
	int midPointRight = 0;

	float xDiff = 0;
	float yDiff = 0;
	float angle = 0;

	float startControlX = 0;
	float startControlY = 0;

	float endControlX = 0;
	float endControlY = 0;

	float startX = 0;
	float startY = 0;

	float endX = 0;
	float endY =0;

	int segments = 25;

	sf::VertexArray v(sf::LinesStrip , 0);
	std::vector<sf::Vector2f> points;

	transColor = generatePleasingColour(sf::Color(255, 255, 255, 255/*rand() % 255, rand() % 255, rand() % 255, 255*/));

	if(startPoint == endPoint)
	{
		if(startPoint->getRightMostSib() == nullptr)
		{
			//loop is centered on the mid point of the circle, and extends to radius * 3
			startControlX = startPoint->getXPos()+RADIUS + (RADIUS * 3)  + yAlignIncrement;
			startControlY = startPoint->getDepth()+RADIUS*2 + xAlignIncrement;

			endControlX = endPoint->getXPos()+RADIUS + (RADIUS*3)  + yAlignIncrement;
			endControlY = endPoint->getDepth() + xAlignIncrement;

			startX = startPoint->getXPos()+RADIUS+yAlignIncrement;
			startY = startPoint->getDepth()+RADIUS*2+xAlignIncrement;

			endX = endPoint->getXPos()+RADIUS+yAlignIncrement;
			endY = endPoint->getDepth()+xAlignIncrement;
	
		}
		else if (startPoint->getLeftMostSib() == nullptr)
		{
			//loop is centered on the mid point of the circle, and extends to radius * 3
			startControlX = startPoint->getXPos()+RADIUS - (RADIUS * 3)  + yAlignIncrement;
			startControlY = startPoint->getDepth()+RADIUS*2 +xAlignIncrement;

			endControlX = endPoint->getXPos()+RADIUS - (RADIUS*3) + yAlignIncrement;
			endControlY = endPoint->getDepth() + xAlignIncrement;

			startX = startPoint->getXPos()+RADIUS+yAlignIncrement;
			startY = startPoint->getDepth()+RADIUS*2+xAlignIncrement;

			endX = endPoint->getXPos()+RADIUS+yAlignIncrement;
			endY = endPoint->getDepth()+xAlignIncrement;
		
		}
		else
		{
			//loop is centered on the mid point of the circle, and extends to radius * 3
			startControlX = startPoint->getXPos() + yAlignIncrement;
			startControlY = startPoint->getDepth() + RADIUS + (RADIUS * 3) + xAlignIncrement;

			endControlX = endPoint->getXPos() + RADIUS * 2  + yAlignIncrement;
			endControlY = endPoint->getDepth() + RADIUS + (RADIUS*3) + xAlignIncrement;

			startX = startPoint->getXPos()+yAlignIncrement;
			startY = startPoint->getDepth()+RADIUS+xAlignIncrement;

			endX = endPoint->getXPos()+RADIUS*2+yAlignIncrement;
			endY = endPoint->getDepth()+RADIUS+xAlignIncrement;
		}
	}
	else
	{
		if(startPoint->getDepth() < endPoint->getDepth())
		{
			startControlX = startPoint->getXPos()+RADIUS+yAlignIncrement;
			startControlY = startPoint->getDepth()+RADIUS+xAlignIncrement;

			endControlX = endPoint->getXPos()+RADIUS+yAlignIncrement;
			endControlY = endPoint->getDepth()+RADIUS+xAlignIncrement;

			startX = startPoint->getXPos()+RADIUS+yAlignIncrement;
			startY = startPoint->getDepth()+RADIUS+xAlignIncrement;

			endX = endPoint->getXPos()+RADIUS+yAlignIncrement;
			endY = endPoint->getDepth()+RADIUS+xAlignIncrement;

		}
		else if(startPoint->getDepth() == endPoint->getDepth())
		{

			//probs best to add case where its points to a state beyond the direct left or right node

			if(startPoint->getGroupNum() == endPoint->getGroupNum() - 1 || startPoint->getGroupNum() == endPoint->getGroupNum() + 1)
			{
				//basically making it so that the lines dont overlap when pointing to states on the same level
				int spacing = 0;
				if(startPoint->getGroupNum() < endPoint->getGroupNum())
				{
					spacing = -25;
				}
				else
				{
					spacing = 25;
				}

				startControlX = startPoint->getXPos()+RADIUS+yAlignIncrement;
				startControlY = startPoint->getDepth()+RADIUS+spacing+xAlignIncrement;

				endControlX = endPoint->getXPos()+RADIUS+yAlignIncrement;
				endControlY = endPoint->getDepth()+RADIUS+spacing+xAlignIncrement;

				startX = startPoint->getXPos()+RADIUS+yAlignIncrement;
				startY = startPoint->getDepth()+RADIUS+spacing+xAlignIncrement;

				endX = endPoint->getXPos()+RADIUS+yAlignIncrement;
				endY = endPoint->getDepth()+RADIUS+spacing+xAlignIncrement;

			}
			else if (startPoint->getGroupNum() > endPoint->getGroupNum())
			{
				//current x + the distance to the right most x + some extra distance which can be tweaked
				startControlX = startPoint->getXPos()+RADIUS - (((startPoint->getXPos()+RADIUS) -(endPoint->getXPos()+RADIUS)) / 3) +yAlignIncrement;
				startControlY = startPoint->getDepth()+RADIUS + (RADIUS * startPoint->getGroupNum()) +xAlignIncrement;
											
				endControlX = endPoint->getXPos()+RADIUS +  (((startPoint->getXPos()+RADIUS) -(endPoint->getXPos()+RADIUS)) / 3) +yAlignIncrement;
				endControlY = endPoint->getDepth()+RADIUS + (RADIUS * startPoint->getGroupNum()) +xAlignIncrement;

				startX = startPoint->getXPos()+RADIUS+yAlignIncrement;
				startY = startPoint->getDepth()+RADIUS+xAlignIncrement;

				endX = endPoint->getXPos()+RADIUS+yAlignIncrement;
				endY = endPoint->getDepth()+RADIUS+xAlignIncrement;

			}
			else if (startPoint->getGroupNum() < endPoint->getGroupNum())
			{
				//current x + the distance to the right most x + some extra distance which can be tweaked
				startControlX = startPoint->getXPos()+RADIUS + (((startPoint->getXPos()+RADIUS) - (endPoint->getXPos()+RADIUS)) / 3) +yAlignIncrement;
				startControlY = startPoint->getDepth()+RADIUS + (RADIUS * endPoint->getGroupNum()) +xAlignIncrement;
											
				endControlX = endPoint->getXPos()+RADIUS - (((startPoint->getXPos()+RADIUS) - (endPoint->getXPos()+RADIUS)) / 3) +yAlignIncrement;
				endControlY = endPoint->getDepth()+RADIUS + (RADIUS * endPoint->getGroupNum()) +xAlignIncrement;

				startX = startPoint->getXPos()+RADIUS+yAlignIncrement;
				startY = startPoint->getDepth()+RADIUS+xAlignIncrement;

				endX = endPoint->getXPos()+RADIUS+yAlignIncrement;
				endY = endPoint->getDepth()+RADIUS+xAlignIncrement;
			}
		}
		else
		{
			//if depth is greater than the end point
			//the startcontrol points needs to be modified to extend across the x axis as far as he first node in teh group
			if (startPoint->getGroupNum() > (startPoint->getParent()->getTransTable().size() / 2))
			{
				//its on the right hand side 
				if(startPoint->getRightMostSib() != nullptr)
				{
					//current x + the distance to the right most x + some extra distance which can be tweaked
					startControlX = ((startPoint->getXPos()+RADIUS) + ((startPoint->getRightMostSib()->getXPos()+ RADIUS - startPoint->getXPos()+RADIUS)
						+ (RADIUS * (startPoint->getParent()->getTransTable().size() - startPoint->getGroupNum())))) + yAlignIncrement;
					startControlY = startPoint->getDepth()+RADIUS + (RADIUS*3);
												
					endControlX = startControlX;
					endControlY = endPoint->getDepth()+RADIUS - ((endPoint->getDepth()+RADIUS - startPoint->getDepth()+RADIUS) / 2) + xAlignIncrement;
				}
				else
				{
					startControlX = (startPoint->getXPos()+RADIUS + (RADIUS * 3)) + yAlignIncrement;
					startControlY = startPoint->getDepth()+RADIUS + ((endPoint->getDepth()+RADIUS - startPoint->getDepth()+RADIUS) / 2) +xAlignIncrement;

					endControlX = startControlX;
					//endControlX = endPoint->getXPos()+RADIUS + ((startPoint->getXPos()+RADIUS -  startPoint->getXPos()+RADIUS) / 2) + (RADIUS * 2) +yAlignIncrement;
					endControlY = endPoint->getDepth()+RADIUS - ((endPoint->getDepth()+RADIUS - startPoint->getDepth()+RADIUS) / 2) + xAlignIncrement;
				}
			}
			else
			{
				if(startPoint->getLeftMostSib() != nullptr)
				{
					// current x - the distance to the left most x - some extra distance which can be tweaked
					startControlX = ((startPoint->getXPos()+RADIUS) - (startPoint->getXPos()+RADIUS - startPoint->getLeftMostSib()->getXPos()+RADIUS)) +yAlignIncrement
						- (RADIUS * startPoint->getLeftMostSib()->getGroupNum()*3);
					startControlY = endPoint->getDepth()+RADIUS +xAlignIncrement - (RADIUS*3);

					endControlX = startControlX;
					endControlY = endPoint->getDepth()+RADIUS + ((endPoint->getDepth()+RADIUS - startPoint->getDepth()+RADIUS) / 2) + xAlignIncrement;
				}
				else
				{
					//is the left most
					startControlX = (startPoint->getDepth()+RADIUS) - (RADIUS * startPoint->getGroupNum()*3) + yAlignIncrement;
					startControlY = startPoint->getDepth()+RADIUS + ((endPoint->getDepth()+RADIUS - startPoint->getDepth()+RADIUS) / 2) +xAlignIncrement;

					endControlX = startControlX;
					endControlY = endPoint->getDepth()+RADIUS + ((endPoint->getDepth()+RADIUS - startPoint->getDepth()+RADIUS) / 2) + xAlignIncrement;
				}
			}

			startX = startPoint->getXPos()+RADIUS+yAlignIncrement;
			startY = startPoint->getDepth()+RADIUS+xAlignIncrement;

			endX = endPoint->getXPos()+RADIUS+yAlignIncrement;
			endY = endPoint->getDepth()+RADIUS+xAlignIncrement;
		}
	}

	points = calcCubicBezier(
		sf::Vector2f(startY, startX),
		sf::Vector2f(endY, endX),
		sf::Vector2f(startControlY, startControlX),
		sf::Vector2f(endControlY, endControlX),
		segments);

	for(std::vector<sf::Vector2f>::const_iterator it = points.begin(); it != points.end(); ++it)
	{
		v.append(sf::Vertex(*it, transColor));
	}

	vertices = v;

	//draw text
	conditionStatement.setFont(*f);
	conditionStatement.setString(startPoint->buildTransCopy(condition)); 
	conditionStatement.setCharacterSize(12);
	conditionStatement.setColor(transColor);
	//set the origin to the center of the text bounding box in  x axis
	conditionStatement.setOrigin(conditionStatement.getOrigin().x + conditionStatement.getLocalBounds().width/2, conditionStatement.getOrigin().y);

	midPointLeft = (int)(points.size())/2;
	conditionStatement.setPosition(points.at(midPointLeft).x , points.at(midPointLeft).y + 20); //.x + 10, position.y+RADIUS);

	//draw arrowhead
	//calc angle between states at the end of the line
	xDiff = endPoint->getXPos() - startPoint->getXPos();
	yDiff = endPoint->getDepth() - startPoint->getDepth();
	//angle is in degrees
	angle = atan2(xDiff, yDiff) * 180.f / (4.f*atan(1.f));

	arrowHead.setRadius(10);
	arrowHead.setPointCount(3);
	//put the origin of the shape in the centre on the x axis
	arrowHead.setOrigin(arrowHead.getOrigin().x + arrowHead.getRadius(), arrowHead.getOrigin().y+ arrowHead.getRadius());
	// hack
	arrowHead.setRotation(angle+90);
	arrowHead.setPosition(points.at(points.size()/2).x, points.at(points.size()/2).y);
	arrowHead.setFillColor(transColor);
}

std::vector<sf::Vector2f> TransitionUI::calcCubicBezier(const sf::Vector2f &start, const sf::Vector2f &end, const sf::Vector2f &startControl, const sf::Vector2f &endControl, const size_t numSegments)
{
	std::vector<sf::Vector2f> ret;
	if (!numSegments) // Any points at all?
		return ret;

	ret.push_back(start); // First point is fixed
	float p = 1.f / numSegments;
	float q = p;
	for (size_t i = 1; i < numSegments; i++, p += q) // Generate all between
		ret.push_back(p * p * p * (end + 3.f * (startControl - endControl) - start) +
						3.f * p * p * (start - 2.f * startControl + endControl) +
						3.f * p * (startControl - start) + start);
	ret.push_back(end); // Last point is fixed`
	return ret;
}

void TransitionUI::redraw(sf::RenderWindow* window, int myStateIndex, int currentStateIndex)
{
	//draw line
	if(myStateIndex == currentStateIndex)
	{
		for(unsigned int idx = 0; idx < vertices.getVertexCount(); idx++)
		{
			if(vertices[idx].color != sf::Color::White)
			{
				vertices[idx].color = sf::Color::White;
			}
			else
			{
				vertices[idx].color = transColor;
			}
		}

		if(arrowHead.getFillColor() != sf::Color::White)
		{
			arrowHead.setFillColor(sf::Color::White);
		}
		else
		{
			arrowHead.setFillColor(transColor);
		}
	}
	else
	{
		if(vertices[0].color == sf::Color::White)
		{
			for(unsigned int idx = 0; idx < vertices.getVertexCount(); idx++)
			{
				vertices[idx].color = transColor;
			}
		}
		if(arrowHead.getFillColor() == sf::Color::White)
		{
			arrowHead.setFillColor(transColor);
		}
	}

	window->draw(vertices);
	//draw text
	window->draw(conditionStatement);
	//draw arrow
	window->draw(arrowHead);
}

sf::Color TransitionUI::generatePleasingColour(sf::Color randCol)
{
	int red = rand() % 255;
	int green = rand() % 255;
	int blue = rand() % 255;

	red = (red + randCol.r) / 2;
	green = (green + randCol.g) / 2;
	blue = (blue + randCol.b) / 2;

	sf::Color mixedColor(red, green, blue , 255);
	return mixedColor;
}

RealTimeDataUI::RealTimeDataUI(sf::RenderWindow* win, std::string dataStream, std::map<std::string, double> channelSet, float xPos, sf::Font* f)
{
	std::string ChanName;
	window = win;

	backGroundRect = sf::RectangleShape(sf::Vector2f(200, SCREENHEIGHT));
	backGroundRect.setFillColor(sf::Color(3, 14, 49, 127));
	backGroundRect.setPosition(xPos, 0);

	streamTitle.setFont(*f);
	streamTitle.setString(dataStream);
	streamTitle.setCharacterSize(20);
	streamTitle.setColor(sf::Color::White);
	streamTitle.setPosition(xPos + 10, 10);

	float yPos = 50;

	for(std::map<std::string, double>::iterator it = channelSet.begin(); it != channelSet.end(); ++it)
	{
		std::stringstream ss;
		ss << it->second;

		sf::Text channelData;
		channelData.setFont(*f);

		if(it->first.size() > 12)
		{
			std::string ChanName = it->first.substr(0, 11);
			ChanName += "..";
		}
		else
		{
			ChanName = it->first;
		}

		channelData.setString(ChanName + " = " + ss.str());
		channelData.setCharacterSize(14);
		channelData.setColor(sf::Color::White);
		channelData.setPosition(xPos + 10, yPos);

		yPos += 30;

		textSet.push_back(channelData);
	}
}

void RealTimeDataUI::redraw(std::map<std::string, double> channelSet)
{
	std::string ChanName;

	window->draw(backGroundRect);
	window->draw(streamTitle);

	for(std::pair<std::vector<sf::Text>::iterator, std::map<std::string, double>::iterator>
		it(textSet.begin(), channelSet.begin()); it.first != textSet.end(); ++it.first, ++it.second)
	{
		std::stringstream ss;
		ss << it.second->second;

		if(it.second->first.size() > 12)
		{
			ChanName = it.second->first.substr(0, 11);
			ChanName += "..";
		}
		else
		{
			ChanName = it.second->first;
		}

		it.first->setString(ChanName + " = " + ss.str());
		
		window->draw(*it.first);
	}
}

