This is the code I wrote for a path planner. I chose to include it because it's my implementation of an important algorithm, A*, and it shows work with data structures, such as a priority queue. The program has a few available tilemaps to path plan through. On the right, you can set the start and end corrdinates, as well as the number of times to run the algorithm. The console window will output information about how long it takes to run the planner. The play button runs through the path planning as an animation, the second button steps through the process one step at a time, and the final button skips the animations and just runs the algoritm. The more shaded a tile is, the more expensive it is to pass through. Blue tiles have been visitied by A*, and green tiles have been added to the priority queue.

Unlike other projects provided in this portfolio, instead of being able to compile and run this project I've included some source code and a binary. This is because the code I wrote, the path planning algorithm, worked with a larger GUI that was provided. Rather than include a bunch of code that isn't mine in my portfolio, I elected to just include the parts that I wrote. In order to still get a feel for what the path planner was like, I included the executable (and two .dll files needed for it to run) already compiled.  