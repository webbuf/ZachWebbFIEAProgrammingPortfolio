#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include "Board.h"
#include "Random.h"
#include "TextureManager.h"
#include "Tile.h"
//#include "Button.h"

int main()
{

    std::ifstream config;
    config.open("../../boards/config.cfg", std::ifstream::in);
    if(!config.is_open()){
        std::cout << "Config file not found. Terminating.";
        exit(0);
    }
    string tileWidthStr, tileHeightStr, mineCountStr;
    //! Added a check to make sure the config file is actually there before we read it
    //! Before, an missing config meant an instant crash with no indication as to what was wrong

    getline(config, tileWidthStr);
    getline(config, tileHeightStr);
    getline(config, mineCountStr);

    config.close(); 

    int tileWidth = stoi(tileWidthStr);
    int tileHeight = stoi(tileHeightStr);
    int mineCount = stoi(mineCountStr); 

    if (mineCount >= (tileWidth * tileHeight) - 9){   
        std::cout << "Invalid config parameters: too many mines. Terminating.";
        exit(0);
    }
    //!This is kind of a silly edge case, but if we give it invalid config paramters don't attempt to continue with them
    //!Namely, we need to have enough clear tiles for at least the 9 around where you click to be safe to actually generate a game

    int pixelWidth = tileWidth * 32;
    int pixelHeight = (tileHeight * 32) + 88;       //extrapolates size of window from tile count using the size of the tile sprite

    Board board(tileWidth, tileHeight, mineCount);
    sf::RenderWindow window(sf::VideoMode(pixelWidth, pixelHeight), "Minesweeper");

    Board::Button buttons[5] = { Board::Button('R', TextureManager::GetTexture("face_happy"), (pixelWidth / 2 - 32), pixelHeight - 88),
                          Board::Button('D', TextureManager::GetTexture("debug"), (pixelWidth / 2 + 32 + 64), pixelHeight - 88),
                          Board::Button('1', TextureManager::GetTexture("test_1"), (pixelWidth / 2 + 32 + 64 * 2), pixelHeight - 88),
                          Board::Button('2', TextureManager::GetTexture("test_2"), (pixelWidth / 2 + 32 + 64 * 3), pixelHeight - 88),
                          Board::Button('3', TextureManager::GetTexture("test_3"), (pixelWidth / 2 + 32 + 64 * 4), pixelHeight - 88)
    };
    /*! The last two variable here are the position of the buttons. While you can tell that from the function declaration,
        I should probably have acknowledged that's what going on in some way here as well. Just looking at main it all feels pretty arbitrary !*/
    //initialize needed buttons

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
            case sf::Event::Closed:
                window.close();
                break;

            case sf::Event::MouseButtonPressed:
                if (event.mouseButton.y < tileHeight * 32) {    //make sure click was in the bounds of the board
                    if (event.mouseButton.button == sf::Mouse::Left) board.OnClick(event.mouseButton.x, event.mouseButton.y, "LEFT", buttons[0]);
                    else board.OnClick(event.mouseButton.x, event.mouseButton.y, "RIGHT", buttons[0]);
                }
                else {
                    for (int i = 0; i < 5; i++) {
                        if (buttons[i].GetPosition().contains(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y)) {
                            buttons[i].ButtonClick(&board, mineCount, buttons[0]);
                        }
                    }
                }
                /*! When clicking on a tile, we use your x and y position to determine which tile you're in
                    While with buttons, we loop through all of them and check if you're in their bounds. Since
                    buttons have variable positions I think it was valid to do it like this, especially since
                    I don't actually store the button's positions anywhere once they're made */
                    
                break;

            default:
                break;
            //! A switch doesn't feel entirely necessary for only two cases, but it does make it easy to scale
            //! So I think it was a good call
            }
        }
        window.clear();
        board.Draw(window);
        for (int i = 0; i < 5; i++) buttons[i].Draw(window);
        window.display();
    }

    TextureManager::Clear();    //manual destruction of texture manager to clear up all the textures out of memory
    return 0;
}