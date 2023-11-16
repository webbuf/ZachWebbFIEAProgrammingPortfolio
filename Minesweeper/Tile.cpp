#include "Tile.h"

void Tile::Reveal(int* score) {
	if (flagged == false) {
		revealed = true;
		if (adjacentMines == 0 && mined == false) {
			for (unsigned int i = 0; i < adjacentTiles.size(); i++)
				if(adjacentTiles[i]->GetRevealed() == false) adjacentTiles[i]->Reveal(score);	//if we don't check it gets stuck in an infinite loop of trying to reveal the same tiles
		}
		if(!mined) *score -= 1;
	}
}

void Tile::SetMine() {
	mined = true;
}

//! This is weird. Should be SetMine(bool) mined = bool, even if I'd always pass in true. 

bool Tile::ReverseFlag() {	//used when player flags tile
		flagged = !flagged;	//doesn't check if tile is revealed as board handles it
		return flagged;		//means always reutrns
}

void Tile::SetFlag() {		//used to flag remaining mines when game is won
	flagged = true;
}

bool Tile::GetRevealed() {
	return revealed;
}
bool Tile::GetMined() {
	return mined;
}
bool Tile::GetFlagged() {
	return flagged;
}

void Tile::AddAdjacentTile(Tile* tile) {
	adjacentTiles.push_back(tile);
}

void Tile::GetAdjacentMines() {
	for (unsigned int i = 0; i < adjacentTiles.size(); i++) {
		if (adjacentTiles[i]->GetMined() == true) adjacentMines++;
	}
	if (adjacentMines != 0) {
		string filename = "number_1";
		char mineChar = '0' + adjacentMines;	//increments value on 0 char by enough to be char same as int value
		filename[7] = mineChar;
		sprites["number"].setTexture(TextureManager::GetTexture(filename));
	}
}

void Tile::SetPosition(int y, int x) {
	xPos = x * 32;
	yPos = y * 32;	//convert tile index to screen position
	auto iter = sprites.begin();
	for (; iter != sprites.end(); iter++) {
		iter->second.setPosition(xPos, yPos);
	}
}

void Tile::Draw(sf::RenderWindow& window, bool& debug) {
	if (!revealed) {
		window.draw(sprites["hidden"]);
		if (flagged) window.draw(sprites["flag"]);
	}
	else {
		window.draw(sprites["revealed"]);
		if (mined) window.draw(sprites["mine"]);
		else if(adjacentMines != 0) window.draw(sprites["number"]);
	}
	if(mined == true && debug == true) window.draw(sprites["mine"]);
}

void Tile::Reset() {
	revealed = false;
	mined = false;
	flagged = false;
	adjacentMines = 0;
}
//return tile to default constructed state
//the bools don't have individual set to false functions because aside from switching states with manual flagging they'll only be set to false at the start of a new game

//! The setters still should have taken a bool as an argument and set it to whatever the argument was. Setters that only make things true is an
//! odd architecture choice, even if setting them to false with the setter never comes up. 

Tile::Tile() {
	flagged = false;
	mined = false;
	revealed = false;
	adjacentMines = 0;
	xPos = 0;
	yPos = 0;

	sf::Sprite hidden;
	sf::Sprite revealed;
	sf::Sprite number;
	sf::Sprite flag;
	sf::Sprite mine;

	hidden.setTexture(TextureManager::GetTexture("tile_hidden"));
	revealed.setTexture(TextureManager::GetTexture("tile_revealed"));
	number.setTexture(TextureManager::GetTexture("number_1"));
	flag.setTexture(TextureManager::GetTexture("flag"));
	mine.setTexture(TextureManager::GetTexture("mine"));

	sprites.emplace("hidden", hidden);
	sprites.emplace("revealed", revealed);
	sprites.emplace("number", number);
	sprites.emplace("flag", flag);
	sprites.emplace("mine", mine);

	/*! When I freed this from the ide this became an error and I had to separate this out into 3 separate 
	    statements per sprite like this. I'm not sure why, since the board buttons still work as these did with no 
	    issue. It's odd but not really a big deal.

	/*! The logic behind having so many sprites for one tile is because they can be drawn on top of each other.
		While hidden and revealed will never be on at the same, you can have a mine, flag, and tile background all
		at once. Similarly, a revealed tile and the number on it are separate. At first glance it looks wasteful, 
		but I think it's a good way of doing it. Having separate hidden and revealed sprites doesn't make a ton of
		sense as they're mutually exclusive. A better appraoch would've been to have a "background" sprite I set to
		the hidden and change the texture to the revealed in the reveal function. !*/
}