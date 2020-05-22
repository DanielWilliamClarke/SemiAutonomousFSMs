//////////////////////////////////////////////////////
// author Daniel Clarke 2013/14

#ifndef CONTROLLER_UI_H
#define CONTROLLER_UI_H

#include <chrono>
#include <time.h>
#include <SFML\Graphics.hpp>

#include "dfa.h"

#define SCREENWIDTH 1300
#define SCREENHEIGHT 675
#define RADIUS 50

class StateUI;
class TransitionUI;

class ControllerUI
{
public:

	ControllerUI(stateVec, sf::RenderWindow* win, sf::Font*);
	void redrawAll(int, std::vector<int>);
	State* manualSelectState(sf::Vector2f, std::vector<int>);
	void stateCollision(sf::Vector2f, sf::Font*);
private:

	std::vector<sf::Text> buildOutputDisplay(State_Output*, sf::Vector2f, sf::Font*);
	float calcYIncrement(State_Output*);
	float calcRectXLength(std::vector<sf::Text>);
	sf::RenderWindow* window;
	std::vector<StateUI*> uiStates;
	std::vector<StateUI*> selectableStates;
	float alignIncrement;
};

class StateUI
{
public:

	StateUI(State*, float, sf::Font*);
	int getIndex();
	State* getMyState();
	sf::CircleShape getStateCircle();
	void redrawStates(sf::RenderWindow*, int,  std::vector<int>);
	void redrawTrans(sf::RenderWindow*, int);
private:
	
	State* myState;
	int index;
	int points;
	sf::Text stateNameText;
	sf::CircleShape state;
	//for bezier curves
	std::vector<TransitionUI*> transitions;
};

class TransitionUI
{
public:

	TransitionUI(State*, State*, Trans_Input, float, float, sf::Font*);
	void redraw(sf::RenderWindow*, int, int);
private:

	sf::Color transColor;
	std::vector<sf::Vector2f> calcCubicBezier(const sf::Vector2f &start, const sf::Vector2f &end, const sf::Vector2f &startControl, const sf::Vector2f &sendControl, const size_t numSegments);
	sf::VertexArray vertices;
	sf::CircleShape arrowHead;
	sf::Text conditionStatement;
	sf::Color generatePleasingColour(sf::Color);
};

class RealTimeDataUI
{
public:

	RealTimeDataUI(sf::RenderWindow*, std::string, std::map<std::string, double>, float xPos, sf::Font*);
	void redraw(std::map<std::string, double>);
private:

	sf::RectangleShape backGroundRect;
	sf::Text streamTitle;
	std::vector<sf::Text> textSet;
	sf::RenderWindow* window;
};

#endif //CONTROLLER_UI_H