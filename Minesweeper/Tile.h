#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include "TextureManager.h"

using std::vector;
using std::unordered_map;

class Tile{
	bool revealed;
	bool mined;
	bool flagged;
	int adjacentMines;
	int xPos;
	int yPos;
	vector<Tile*> adjacentTiles;
	unordered_map<string, sf::Sprite> sprites; 

public:
	void Reveal(int* score);
	void SetMine();
	bool ReverseFlag();
	void SetFlag();
	bool GetRevealed();
	bool GetMined();
	bool GetFlagged();
	void AddAdjacentTile(Tile* tile);
	void GetAdjacentMines();
	void SetPosition(int y, int x);
	void Draw(sf::RenderWindow& window, bool& debug);
	void Reset();
	Tile();
};

