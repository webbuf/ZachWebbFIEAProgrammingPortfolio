#pragma once
#include "Tile.h"
#include "Random.h"
#include "TextureManager.h"
//#include "Button.h"  used to be a separate file, turned into nested class after thing weren't working properly
#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include <streambuf>
#include <unordered_map>
#include <SFML/Graphics.hpp>

using std::vector;
using std::ifstream;
using std::string;

class Board{
	int width;
	int height;
	int mineCount;	//stores current game's mine count
	int minesToGenerate;	//!stores how many mines to generate in a random game
	//! There was a bug in this as I submitted it. Loading a debug board would change
	//! the amount of mines futures games would generate to however many mines were on 
	//! the debug board. Separating out a variable to remember the mine count config file remedies this. 
	int neededFlagCount;
	bool debug;
	bool tempDebug;
	bool gameOngoing;
	bool gameStarted;
	int score;

	vector<vector<Tile>> tiles;		//! This should be Tile pointers as well
	vector<Tile*> minedTiles;

	unordered_map<int, sf::Sprite> flagCounter = { {1, sf::Sprite(TextureManager::GetTexture("digits"), sf::IntRect(0, 0, 21, 32))},
												   {10, sf::Sprite(TextureManager::GetTexture("digits"), sf::IntRect(0, 0, 21, 32))},
												   {100, sf::Sprite(TextureManager::GetTexture("digits"), sf::IntRect(0, 0, 21, 32))},
												   {-1, sf::Sprite(TextureManager::GetTexture("digits"), sf::IntRect(210, 0, 21, 32))},
	};
	/*! I chose to make the flag counter an unordered map so which place I was modifying was more clear. 
	Since it's unordered there shouldn't be any time hit compared to keeping these in a vector, so it's
	a choice I stand by. Another option could have been to define ONES TENS and HUNDREDS and 0, 1, and 2, 
	and use those to access just a vector. Either way, I think it's fine. !*/

public:
	struct Button {
		char id;
		sf::Sprite sprite;

		void ButtonClick(Board* board, int totalMines, Button& button);
		void Draw(sf::RenderWindow& window);
		sf::FloatRect GetPosition();
		void SetSprite(sf::Texture& texture);
		Button(char id, sf::Texture& texture, int xPos, int yPos);
	};

	Board(int width, int height, int mineCount);
	void MineInitializeRandom(int totalMines, int x, int y);
	void MineInitializeDebug(string file);
	void SetAdjacentTiles();
	void SetAdjacentMines();
	void Draw(sf::RenderWindow& window);
	void OnClick(int MouseX, int MouseY, string clickType, Button& button);
	void Reset(Button& button);
	void FlipDebug();
	bool GameStatus();
	void UpdateFlagCounter();
};

