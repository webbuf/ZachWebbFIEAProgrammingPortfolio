#include "PathSearch.h"

using ufl_cap4053::Tile;
using ufl_cap4053::TileMap;
using ufl_cap4053::PriorityQueue;
using std::vector;
using std::unordered_map;
using std::cout;
using std::endl;

namespace ufl_cap4053
{
	namespace searches
	{
		PathSearch::SearchNode::SearchNode(Tile* t) {
			vertex = t;
		}

		Tile* PathSearch::SearchNode::getTile() const {
			return vertex;
		}

		void PathSearch::SearchNode::addNeighbor(PathSearch::SearchNode* n) {
			neighbors.push_back(n);
		}

		vector<PathSearch::SearchNode*>& PathSearch::SearchNode::getNeighbors() {
			return neighbors;
		}

		PathSearch::PlannerNode::PlannerNode(PathSearch::SearchNode* v, PathSearch::PlannerNode* p, float heuristic, float unitLength) {
			vertex = v;
			parent = p;
			//Tile* currentTile = v->getTile();
			//heuristic = sqrt(pow(goalRow - currentTile->getYCoordinate(), 2) + pow(goalCol - currentTile->getXCoordinate(), 2));
			//since we won't need to remember just the heuristic for anything, we instead calculate it outside and pass in the result, since there's no need to store it
			//we could still pass in the goal coords and stuff and find it in here, but it's cleaner with 1 parameter instead of 2 IMO
			givenCost = p->getCost() + (getTile()->getWeight() * unitLength);
			//we need to remember the given cost, since the paren't influences the child's
			//given cost is cost to get to parent plus the weight to get here
			totalCost = heuristic * HEURISTIC_WEIGHT + givenCost;
		}

		PathSearch::PlannerNode::PlannerNode(PathSearch::SearchNode* v, float heuristic) {
			vertex = v;
			parent = nullptr;
			Tile* currentTile = v->getTile();
			//heuristic = sqrt(pow(goalRow - currentTile->getYCoordinate(), 2) + pow(goalCol - currentTile->getXCoordinate(), 2));
			//multiplies the straight line distance to the goal by the weight of the current tile
			givenCost = 0;
			//since this is the root, no cost so far
			totalCost = heuristic;
			//since our cost is 0, the A* cost is just the heuristic
		}

		Tile* PathSearch::PlannerNode::getTile() const {
			return vertex->getTile();
		}

		PathSearch::SearchNode* PathSearch::PlannerNode::getNode() const {
			return vertex;
		}

		PathSearch::PlannerNode* PathSearch::PlannerNode::getParent() const {
			return parent;
		}

		/* float PathSearch::PlannerNode::getHeuristic() const {
			return heuristic;
		} */

		float PathSearch::PlannerNode::getCost() const {
			return givenCost;
		}

		float PathSearch::PlannerNode::getTotalCost() const {
			return totalCost;
		}

		//our search space is a hexagonal grid, so addjacency isn't as simple as up down left right
		bool PathSearch::areAdjacent(const Tile* lhs, const Tile* rhs) {
			if (lhs->getRow() == rhs->getRow() && lhs->getColumn() == rhs->getColumn()) return false; //tile isn't adjacent to itself
			else if (lhs->getRow() % 2 == 0) {
				if (rhs->getColumn() <= lhs->getColumn()) return true; //all tiles to the left and inline with an even column tile are adjacent
				else if (rhs->getRow() == lhs->getRow()) return true;	//tile to the right of lhs but inline is adjacent
				else return false;	//tile to the right and above and to the right and below (only remaining) are not adjacent
			}
			//even row case
			else {
				if (rhs->getColumn() >= lhs->getColumn()) return true; //all tiles to the right and inline with an even column tile are adjacent
				else if (rhs->getRow() == lhs->getRow()) return true;	//tile to the left of lhs but inline is adjacent
				else return false;	//tile to the left and above and to the left and below are not adjacent
			}
			//odd row case
		}
		//since we only call areAdjacent on the nodes directly surrounding a tile, we can make some assumptions
		//namely, that rhs will be one of the 9 tiles in a 3 x 3 grid (of squares) centered on lhs
		//thus, we only need to check if the conditions for a tile in this space to not be adjacent are true
		//since the conditions to be adjacent are immplicity already set in buildSearchGraph
		//this simplifies areAdjacent considerably

		void PathSearch::buildSearchGraph() {
			int rows = tileMap->getRowCount();
			int cols = tileMap->getColumnCount();
			for (int i = 0; i < rows; i++) {
				for (int j = 0; j < cols; j++) {
					Tile* currentTile = tileMap->getTile(i, j);
					SearchNode* newNode = new SearchNode(currentTile);
					searchGraph[currentTile] = newNode;
				}
			}
			//first we go through and add all tiles to the search graph
			//next, we'll construct the actual adjacency
			//we do these separately so a neighboring node will always already be in the search graph
			//we could pass on adding impassible tiles to the search graph at all, but the extra logic to
			//dodge them when we build adjacency would be a pain, probably have to try catch
			//slight memory hit is worth the simplicity of just leaving them connected to nothing

			for (int i = 0; i < rows; i++) {
				for (int j = 0; j < cols; j++) {
					Tile* currentTile = tileMap->getTile(i, j);
					if (currentTile->getWeight() == 0) continue;	//if impassible, nothing's neighbor
					for (int neighborRow = i - 1; neighborRow <= i + 1; neighborRow++) {
						if (neighborRow < 0 || neighborRow >= rows) continue;
						//if we're out of bounds, don't check this row
						for (int neighborCol = j - 1; neighborCol <= j + 1; neighborCol++) {
							if (neighborCol < 0 || neighborCol >= cols) continue;
							//more bounds checking
							//if we get here, the tile actually exists
							Tile* currentNeighbor = tileMap->getTile(neighborRow, neighborCol);
							if (currentNeighbor->getWeight() == 0) continue;	//if the tile is impassible, it's not a neighbor
							if (areAdjacent(currentTile, currentNeighbor)) {
								searchGraph[currentTile]->addNeighbor(searchGraph[currentNeighbor]);
							}
						}
					}
				}
			}
		}

		void PathSearch::searchIteration() {
			PlannerNode* current = searchQueue.front();
			SearchNode* currentSearch = current->getNode();
			Tile* currentTile = current->getTile();
			visited.insert(currentSearch);
			searchQueue.pop();
			currentTile->setFill(0xFF0000FF);
			//get node at front of queue, mark it as visited and remove it from the queue

			if (currentTile->getRow() == endRow && currentTile->getColumn() == endCol) {
				done = true;
				searchFinalize(current);	//helper function to clean up and return from search, pretty much just makes the final path
			}

			else {
				vector<SearchNode*> currentNodeNeighbors = currentSearch->getNeighbors();
				for (int i = 0; i < currentNodeNeighbors.size(); i++) {
					SearchNode* currentNeighbor = currentNodeNeighbors[i];

					if (visited.find(currentNeighbor) != visited.end()) {
						continue;
					}

					//if we've already visited the node, move on. continute before we allocate the new planner as an optimization

					float newNodeHeuristic = sqrt(pow(endY - currentNeighbor->getTile()->getYCoordinate(), 2) + pow(endX - currentNeighbor->getTile()->getXCoordinate(), 2));
					PlannerNode* newNode = new PlannerNode(currentNeighbor, current, newNodeHeuristic, stepSize);
					//as discussed in the planner node constructor, we get the heuristic outside and then let it fall to the wayside since nothing actually depends on 
					//just the heuristic

					if (queuedNodes.find(currentNeighbor) != queuedNodes.end()) {
						//if (newNode->getCost() < queuedNodes[currentNeighbor]->getCost()) {	  //UCS
						if (newNode->getTotalCost() < queuedNodes[currentNeighbor]->getTotalCost()) {   //A*
							//make sure you also change the comparator
							//theoretically if two nodes had what SHOULD be the same cost but due to floating point stuff one was slightly different, this could replace when it doesn't need to
							//such a minor difference shouldn't affected the cost of the path though, so it's fine
							//a check to see if there was a real difference would almost certainly cost most time than it would save in avoiding needless replacements
							searchQueue.remove(queuedNodes[currentNeighbor]);
							delete queuedNodes[currentNeighbor];
							queuedNodes[currentNeighbor] = newNode;
							searchQueue.push(newNode);
							//if a new path to a node is more efficient, deallocate the old node and replace it with the better one
						}
						else {
							delete newNode;
						}
					}
					//if the node is in the queue, check if the new route is cheaper
					//if it is, replace it. 
					//if not, just move on

					else {
						currentNeighbor->getTile()->setFill(0xFF00FF00);
						queuedNodes[currentNeighbor] = newNode;
						searchQueue.push(newNode);
					}

					//if it's a brand new node, put it in the queue

					/* if (visited.find(currentNodeNeighbors[i]) == visited.end()) {
						searchQueue.push(new PlannerNode(currentNodeNeighbors[i], current));
						visited.insert(currentNodeNeighbors[i]);
						currentNodeNeighbors[i]->getTile()->setFill(0xFF00FF00);
					} */
					//BFS iteration
				}
			}
		}

		void PathSearch::searchFinalize(PathSearch::PlannerNode* endpoint) {
			PlannerNode* current = endpoint;
			while (current) {
				finalPath.push_back(current->getTile());
				if (current->getParent() != nullptr) {
					current->getTile()->addLineTo(current->getParent()->getTile(), 0xFFFF0000);
					current = current->getParent();
				}
				else break;
			}
			//builds the final path vector and draws the line from start to end
			return;
		}

		bool PathSearch::isGreaterThan(PlannerNode* const& lhs, PlannerNode* const& rhs) {
			//return lhs->getHeuristic() > rhs->getHeuristic();		//GBFS
			//return lhs->getCost() > rhs->getCost();		//UCS
			return lhs->getTotalCost() > rhs->getTotalCost(); //A*

			//make sure you also change the duplicate check
		}

		PathSearch::PathSearch() : searchQueue(isGreaterThan) {
			tileMap = nullptr;
			beginRow = 0;
			beginCol = 0;
			endRow = 0;
			endCol = 0;
			done = false;
		}

		PathSearch::~PathSearch() {
			shutdown();
			unload();
		}
		//clean up whatever memory is still allocated when we kill the path search
		//together, these two clean up everything
		//theoretically everything should be deallocated before this gets called?
		//but it can't hurt, and it will clean up everything. So no reason not to

		void PathSearch::load(ufl_cap4053::TileMap* _tilemap) {
			tileMap = _tilemap;
			stepSize = tileMap->getTileRadius() * 2;
			//distance to go from one tile to an adjacent is always 2 * radius, since we go center to center
			buildSearchGraph();
			return;
		}

		void PathSearch::initialize(int startRow, int startCol, int goalRow, int goalCol) {
			//shutdown();	//issues with running a timed go multiple times in a row, this helps clean it up
			//actual issue comes from having text highlighted in the console output, I believe. Not actually from things not deallocating
			//since shutdown is called right after the search when we do a timed run. When it comes to the crash from the conole, I'm
			//not sure if that's on us. wouldn't even be able to begin to understand how to fix that, so hopefully it isn't :P
			//was a source of some slowdown, enough to get outpaced on searches over a small space
			//shutdown should be handled by the larger program, so don't also call it here

			finalPath.clear();		//reset our solution

			beginRow = startRow;
			beginCol = startCol;
			endRow = goalRow;
			endCol = goalCol;
			endY = tileMap->getTile(endRow, endCol)->getYCoordinate();
			endX = tileMap->getTile(endRow, endCol)->getXCoordinate();
			//coordinate endpoints of the goal for heuristics

			done = false;
			float firstHeuristic = sqrt(pow(endY - tileMap->getTile(startRow, startCol)->getYCoordinate(), 2) + pow(endX - tileMap->getTile(startRow, startCol)->getXCoordinate(), 2));
			PlannerNode* firstNode = new PlannerNode(searchGraph[tileMap->getTile(startRow, startCol)], firstHeuristic);
			searchQueue.push(firstNode);
			queuedNodes[searchGraph[tileMap->getTile(startRow, startCol)]] = firstNode;
			return;
		}

		void PathSearch::update(long timeslice) {
			auto t1 = std::chrono::system_clock::now();
			auto t2 = std::chrono::system_clock::now();
			//declare t2 pre-loop so we only have to initiallize it once
			do {
				searchIteration();
				t2 = std::chrono::system_clock::now();
			} while (std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() < timeslice && !done);
			//do while so we always perform at least 1 iteration
			//exit when our timeslice is up, or when we finish
			return;
		}

		void PathSearch::shutdown() {
			for (auto iter = queuedNodes.begin(); iter != queuedNodes.end(); iter++) {
				delete iter->second;
			}
			//queuedNodes holds all the planner nodes we ever use, so we can iterate over it to delete all of them
			//all the nodes that get replaced get deallocated as they get replaced, so no need to worry about that
			queuedNodes.clear();
			visited.clear();
			searchQueue.clear();
			//empty all our containers
			//nothing else gets deallocated, since we're still using the search nodes
		}
		void PathSearch::unload() {
			for (auto iter = searchGraph.begin(); iter != searchGraph.end(); iter++) {
				delete iter->second;
			}
			searchGraph.clear();
			return;
			//deallocate the search graph once we unload the map
			//everything else handled by shutdown, so this is the only container we care about in here
		}

		bool PathSearch::isDone() const { return done; }

		std::vector<Tile const*> const PathSearch::getSolution() const {
			return finalPath;
		};
	}
}