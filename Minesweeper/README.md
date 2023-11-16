This is a Minesweeper game I made for Programming Fundamentals 2. It shows my ability to structure a project and work with a library, in this case SFML.  I've gone back and improved it in a few locations since submitting. Additionally, I've passed over all the code and left retrospective comments analyzing the quality and decision making of the code. New comments are prefixed with a !, comments without were made at the time of creation. 

To build, first in this directory run the following two cmake commands:

``` cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" ```

``` cmake --build build```

Then, cd into the `build` directory and build with MinGW. Typically this command is `mingw32-make` thought in some cases it has been aliased to `make` (though I doubt I need to explain MinGW to you)

The executable can then be found in the `bin` subdirectory of `build`. The first two commands configure CMake and link the project with SFML, and the final builds it. These commands target the compiler as MinGW, the windows wrapper for GCC. If you're on another system or want to target something else, change the generator specified after `-G` in the first command

Minesweeper should speak for itself. Click on a tile to reveal it, right to flag it as having a mine. The smiley face resets the game. The mine button shows all available mines. The three test buttons load three different debug boards. A config file in the boards directory allows you to edit the number of columns, rows, and mines.

Also, I've recorded a demo of me playing the game: https://youtu.be/QBq_JLd2w6I

I moved this project out of an ide at a later date using CMake. I based my CMakeLists.txt off the one found in this github: https://github.com/SFML/cmake-sfml-project