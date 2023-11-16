#include "Board.h"

Board::Board(int width, int height, int mineCount) {
	this->width = width;
	this->height = height;
	this->minesToGenerate = mineCount;
	this->mineCount = minesToGenerate;
	debug = false;
	tempDebug = false;
	score = 1;
	gameOngoing = true;
	gameStarted = false;
	neededFlagCount = mineCount;
	UpdateFlagCounter();
	//initialize variables

	flagCounter[1].setPosition(63, height * 32);
	flagCounter[10].setPosition(42, height * 32);
	flagCounter[100].setPosition(21, height * 32);
	flagCounter[-1].setPosition(0, height * 32);
	//put counter in right spot

	tiles.resize(height);
	for (unsigned int i = 0; i < tiles.size(); i++) {
		tiles[i].resize(width);
	}
	//fill board with default constructed tiles

	SetAdjacentTiles();
	//MineInitializeRandom(mineCount);
}

void Board::MineInitializeRandom(int totalMines, int x, int y) {
	for (int i = 0; i < totalMines;) {		
		//! a for loop with no iterate condition looks weird, but I only want to count it as an iteration if we actually made a mine
		int yCord = Random::Number(0, height - 1);
		int xCord = Random::Number(0, width - 1);
		Tile* currentTile = &tiles[yCord][xCord];
		if (currentTile->GetMined() == false && !((abs(xCord - x) <= 1) && (abs(yCord - y) <= 1))) {
			/*! This ensures that the 3 x 3 grid around the tile we're generating around won't have any mines in it.
			This way, the player will always be given a larger exposed area to work with when beginning the game, so 
			They don't just have one tile to work with. Doing it like this also ensure the player will never start the 
			game by clicking on a mine and getting an instant game over. !*/
			currentTile->SetMine();
			minedTiles.push_back(currentTile);
			i++;	//only counts as a successful loop if a mine was actually placed, protects from the same tile being randomly called twice
		}
	}
	mineCount = totalMines;
	neededFlagCount = mineCount;
	score = (width * height) - mineCount;	
	//! Score keeps track of how many unrevealed mines there are so we know when the game ends quickly. Kind of a misnomer since score ticks down, but I think it's fine
	SetAdjacentMines();
	UpdateFlagCounter();
}

void Board::MineInitializeDebug(string file) {
	gameStarted = true;
	mineCount = 0;
	ifstream input(file);
	string boardLayout((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

	/* for (int currentTileY = 0; currentTileY < (int)tiles.size(); currentTileY++) {
		for (int currentTileX = 0; currentTileX < (int)tiles[currentTileY].size(); currentTileX++) {
			int currentStringIndex = (currentTileY * (width + 1)) + currentTileX;	//translate 2 dimentional tile coordinates into 1 dimensional string index
			if (boardLayout[currentStringIndex] == '1') {
				tiles[currentTileY][currentTileX].SetMine();
				mineCount++;
				minedTiles.push_back(&tiles[currentTileY][currentTileX]);
			}
		}
	} */
	//old version, crashed if board was larger than config file
	//new version lets the board be bigger than the config, but not the other way around
	//felt this way was more lenient

	for (int i = 0; i < boardLayout.size(); i++) {
		if (boardLayout[i] == '1') {
			int currentTileY = i / (width + 1);
			int currentTileX = i % (width + 1);
			tiles[currentTileY][currentTileX].SetMine();
			mineCount++;
			minedTiles.push_back(&tiles[currentTileY][currentTileX]);
		}
	}
	neededFlagCount = mineCount;
	score = (width * height) - mineCount;
	SetAdjacentMines();
	UpdateFlagCounter();
	input.close();
}
//function assumes config is set correctly
//! This is probably a poor choice, since the buttons are there no matter what but are only valid for a
//! certain board size, but with the conditions this was made under it's fine

void Board::SetAdjacentTiles() {
	for (int currentTileY = 0; currentTileY < (int) tiles.size(); currentTileY++) {
		for (int currentTileX = 0; currentTileX < (int) tiles[currentTileY].size(); currentTileX++) {
			Tile* currentTile = &tiles[currentTileY][currentTileX];
			//loops through each tile on the board
			for (int adjacentTileY = currentTileY - 1; adjacentTileY <= currentTileY + 1; adjacentTileY++) {
				if (adjacentTileY < 0 || adjacentTileY == height) {
					continue;
				}
				for (int adjacentTileX = currentTileX - 1; adjacentTileX <= currentTileX + 1; adjacentTileX++) {
					if (adjacentTileX < 0 || adjacentTileX == width || (adjacentTileY == currentTileY && adjacentTileX == currentTileX)) {
						continue;
					}
					currentTile->AddAdjacentTile(&tiles[adjacentTileY][adjacentTileX]);
				}
			}
			//loops through the tiles surrounding the current tile. skips if the tile is out of bounds or is the current tile
			//! This looks pretty gross, but we only ever do it once on initializing the board.
			//! I think doing it later I would only have the if statement look for if we're in the same tile,
			//! And otherwise just let a try/catch with a continue in the catch handle it, but it's not a big deal since it just goes once ever. 
			currentTile->SetPosition(currentTileY, currentTileX); //sets tile's sprites to correct position
		}
	}
}
//function only ever needs to be called once on initialization, hence why setting mines is a separate function instead of concurrent

void Board::SetAdjacentMines() {
	for (int currentTileY = 0; currentTileY < tiles.size(); currentTileY++) {
		for (int currentTileX = 0; currentTileX < tiles[currentTileY].size(); currentTileX++) {
			tiles[currentTileY][currentTileX].GetAdjacentMines();
		}
	}
}

void Board::Draw(sf::RenderWindow& window) {
	for (int currentTileY = 0; currentTileY < tiles.size(); currentTileY++) {
		for (int currentTileX = 0; currentTileX < tiles[currentTileY].size(); currentTileX++) {
			tiles[currentTileY][currentTileX].Draw(window, debug);
		}
	}

	window.draw(flagCounter[1]);
	window.draw(flagCounter[10]);
	window.draw(flagCounter[100]);
	if (neededFlagCount < 0) window.draw(flagCounter[-1]);
	//not handled with loop since the negative sign is only drawn sometimes
	/*! Probably the only drawback of the unordered map is having to invoke three separate calls for each digit
	with a vector we could iterate through the first three, but with unordered we have no idea what order
	they're in, so we have to do them separately. I guess I could access 10^i, but that's so ugly. At three calls
	I think doing it like this is totally fine, but if it were more than this I'd try to think of a diffeerent way
	to do it. !*/
}
//wrapper to call draw on all the tiles

void Board::OnClick(int MouseX, int MouseY, string clickType, Button& button) {
	if (gameOngoing) {		//don't interact with board if game's over
		int TileY = MouseY / 32;
		int TileX = MouseX / 32;	//converts mouse click position to an index on the grid of tiles
		if (clickType.compare("LEFT") == 0) {
			if(!gameStarted){
				MineInitializeRandom(minesToGenerate, TileX, TileY);
				gameStarted = true;
			}
			/*! We generate the game on the first click to ensure a good starting position.
			  This was added by me during the comment pass. At time of submission, you could instantly
			  game over. It was an easy fix, but I didn't do it because we didn't need to for full credit.
			  Drawbacks of making a project for credit is you only do what you need to, I guess. !*/

			tiles[TileY][TileX].Reveal(&score);		//! Always reveal on left click. If we click on an old tile nothing happens, and we need to after game generate as well
			if (tiles[TileY][TileX].GetMined() == true) {
				gameOngoing = false;
				tempDebug = debug;		//storage variable for debug allows it to be used for debugging and showing tiles after a loss
				debug = true;
				for (unsigned int i = 0; i < minedTiles.size(); i++) minedTiles[i]->Reveal(&score);
				button.SetSprite(TextureManager::GetTexture("face_lose"));
			}
			if (score == 0) {
				gameOngoing = false;
				tempDebug = debug;
				neededFlagCount = 0;
				UpdateFlagCounter();
				for (unsigned int i = 0; i < minedTiles.size(); i++) minedTiles[i]->SetFlag();
				button.SetSprite(TextureManager::GetTexture("face_win"));
			}
		}
		else if(tiles[TileY][TileX].GetRevealed() == false && gameStarted){
			//! Don't let the player flag if the game hasn't started.
			//! It doesn't make any sense to do it, and it would probably mess with generation
			if (tiles[TileY][TileX].ReverseFlag()) neededFlagCount--;
			else neededFlagCount++;
			UpdateFlagCounter();
		}
	}
}

void Board::Reset(Button& button) {
	for (int currentTileY = 0; currentTileY < tiles.size(); currentTileY++) {
		for (int currentTileX = 0; currentTileX < tiles[currentTileY].size(); currentTileX++) {
			tiles[currentTileY][currentTileX].Reset();
		}
	}
	minedTiles.clear();
	gameOngoing = true;
	gameStarted = false;	
	//! It's kind of odd to have the game ongoing but not started. Dangers of adding a variable
	//! years later and not wanting to touch the existing source more than needed.

	neededFlagCount = minesToGenerate;	//! Despite not actually having mines yet, display the flag counter like we do
	UpdateFlagCounter();
	debug = tempDebug;
	button.SetSprite(TextureManager::GetTexture("face_happy"));
}

void Board::FlipDebug() {
	debug = !debug;
	tempDebug = debug;
}

bool Board::GameStatus() {
	return gameOngoing;
}

void Board::UpdateFlagCounter() {
	int hundredsPosition = abs(neededFlagCount) / 100;
	int tensPosition = abs(neededFlagCount) / 10;
	if (tensPosition > 9) tensPosition = tensPosition % 10;
	int onesPosition = abs(neededFlagCount) % 10;
	//getting digits in hundreds, tens, and ones place
	//absolute value lets it work for positive and negative values, negative handled by draw

	flagCounter[100].setTextureRect(sf::IntRect(21 * hundredsPosition, 0, 21, 32));
	flagCounter[10].setTextureRect(sf::IntRect(21 * tensPosition, 0, 21, 32));
	flagCounter[1].setTextureRect(sf::IntRect(21 * onesPosition, 0, 21, 32));
}
//flag counter only updates when it needs to instead of trying to stay current on every frame

//! If I were to write this today, I would pass in the number of flags as an arguement to this
//! Pretty much everywhere it's invoked I modify the flag counter directly before
//! It would be a lot cleaner to just change the flag count in here

/******************Button Subclass Declarations********************/
void Board::Button::ButtonClick(Board* board, int totalMines, Button& button) {
	if (id == 'R') {
		board->Reset(button);
		//board->MineInitializeRandom(totalMines);
		//! No longer initialize the mines on reset, since we want to avoid instant game over
	}
	else if (id == 'D') {
		if(board->GameStatus()) board->FlipDebug();
	}
	else {
		string filename = "../../boards/testboard0.brd";
		filename[22] = id;
		board->Reset(button);
		board->MineInitializeDebug(filename);
	}
	/*! Storing the number of the board to load as the id and modifying a stock filename
		is fine with the debug boards provided, but it isn't very modular. If I wanted to 
		make my own boards, they would have to follow the testboard[digit] naming scheme.
		It would be cleaner to just store the filename as a string in the button, and
		get it from there. But once again since it's for a project and the boards were given
		to me, I didn't do it that way. Since id is already a character, I wouldn't have
		to change much of anything. A full string as an id is a little odd, but it's still
		a valid way to identify it. Change the name of the vairable to "name" and it'd be
		totally fine. !*/
}

void Board::Button::SetSprite(sf::Texture& texture) {
	sprite.setTexture(texture);
}

Board::Button::Button(char id, sf::Texture& texture, int xPos, int yPos) {
	this->id = id;
	sprite = sf::Sprite(texture);
	sprite.setPosition(xPos, yPos);
}

void Board::Button::Draw(sf::RenderWindow& window) {
	window.draw(sprite);
}

sf::FloatRect Board::Button::GetPosition() {
	return sprite.getGlobalBounds();
}